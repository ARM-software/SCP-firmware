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
                        --target=jenkins docker
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
            }
        }
    }
}
