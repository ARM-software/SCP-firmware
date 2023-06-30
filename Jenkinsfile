@NonCPS
List generateMatrix(Map matrixAxes) {
    List axes = []

    matrixAxes.each { axis, values ->
        List axisList = []

        values.each { value ->
            axisList << [(axis): value]
        }

        axes << axisList
    }

    axes.combinations()*.sum()
}

pipeline {
    agent {
        label 'docker'
    }

    stages {
        stage('Build container') {
            steps {
                /*
                 * Using `--target` with the Dockerfile agent currently bugs out
                 * on Jenkins, producing an error along the lines of:
                 *
                 *     Cannot retrieve .Id from 'docker inspect ...'
                 *
                 * Apparently it doesn't like multi-stage builds, so until
                 * we're on a version of the Docker workflow plugin that
                 * doesn't exhibit this bug, we just have to build the image
                 * ourselves.
                 *
                 * See:
                 *   - https://github.com/jenkinsci/docker-workflow-plugin/pull/149
                 *   - https://github.com/jenkinsci/docker-workflow-plugin/pull/162
                 *   - https://github.com/jenkinsci/docker-workflow-plugin/pull/180
                 *
                 * Once the Docker workflow plugin has been updated, we should
                 * be able to get rid of this stage and use:
                 *
                 *     agent {
                 *         dockerfile {
                 *             dir 'docker'
                 *             filename 'Dockerfile'
                 *             additionalBuildArgs '--target ci'
                 *         }
                 *     }
                 */

                sh """ \
                    docker build -t scp-firmware:build-${currentBuild.number} \
                        --build-arg JENKINS_UID=1000 \
                        --build-arg JENKINS_GID=36293 \
                        --target=jenkins \
                        --file docker/Dockerfile .
                """
            }
        }

        stage('Run tests') {
            parallel {
                stage('Run legacy tests') {
                    agent {
                        docker {
                            image "scp-firmware:build-${currentBuild.number}"
                            args '-e ARMLMD_LICENSE_FILE'
                        }
                    }

                    steps {
                        sh '/usr/local/bin/init'
                        sh 'python3 ./tools/ci.py'
                    }
                }

                stage('Build and test') {
                    /*
                     * We are on an old enough version of the Jenkins pipeline
                     * workflow plugin that we do not have support for matrices.
                     *
                     * Without this support, we need to generate the stages as
                     * part of a scripted stage.
                     *
                     * See:
                     *  - https://www.jenkins.io/blog/2019/11/22/welcome-to-the-matrix/
                     *  - https://www.jenkins.io/blog/2019/12/02/matrix-building-with-scripted-pipeline/
                     *  - https://stackoverflow.com/questions/60829465/jenkins-unknown-stage-section-matrix-in-declarative-pipeline
                     *
                     * Once the plugin has been updated, we can adopt a proper
                     * matrix.
                     */

                    steps {
                        script {
                            /*
                             * This is, admittedly, not particularly clean, but
                             * should require very little adjustment. The
                             * premise is relatively simple: generate a list of
                             * all the possible matrix combinations, create a
                             * a list of closures returning a pipeline stage for
                             * each combination, then execute them in parallel.
                             */

                            def axes = [
                                generator: [ 'Ninja', 'Unix Makefiles' ],
                                buildType: [ 'Debug', 'Release', 'MinSizeRel',
                                             'RelWithDebInfo' ]
                            ]

                            def tasks = generateMatrix(axes).collectEntries { config ->
                                ["Build and test (${config})", {
                                    node('docker') {
                                        docker
                                            .image("scp-firmware:build-${currentBuild.number}")
                                            .inside('-e ARMLMD_LICENSE_FILE')
                                        {
                                            /*
                                             * If you need to adjust the
                                             * behaviour of the generated stages
                                             * then this is probably where you
                                             * want to do it.
                                             */

                                            stage("Build ${config}") {
                                                checkout scm

                                                /*
                                                 * Unfortunately, we don't have
                                                 * the CMake build plugin
                                                 * available to us, so we'll
                                                 * have to make do with shell
                                                 * scripts for now.
                                                 */

                                                sh '/usr/local/bin/init'

                                                sh """ \
                                                    cmake \
                                                        -G "${config.generator}" \
                                                        -DCMAKE_BUILD_TYPE="${config.buildType}" \
                                                        .
                                                """

                                                sh 'cmake --build .'
                                            }

                                            stage("Check ${config}") {
                                                sh """
                                                    cmake
                                                        --build . \
                                                        --target check
                                                """
                                            }
                                        }
                                    }
                                }]
                            }

                            parallel tasks
                        }
                    }
                }
            }
        }
    }
}
