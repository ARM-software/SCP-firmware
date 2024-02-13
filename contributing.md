Contributing to SCP-firmware
============================

Getting Started
---------------

- Make sure you have a [Gitlab account](https://gitlab.com/users/sign_in).
- Clone the repo to your own machine.
- Create a local topic branch based on the
  [SCP-firmware](https://gitlab.arm.com/firmware/SCP-firmware) `main` branch.

Making Changes
--------------

- Make commits of logical units. See these general
  [Git guidelines](http://git-scm.com/book/ch5-2.html) for contributing to a
  project.
- Follow the project [coding style](./doc/code_style.md) and
  [coding rules](./doc/code_rules.md).
- Keep the commits on topic. If you need to fix another bug or make another
  enhancement, please address it on a separate topic branch.
- Avoid long commit series. If you do have a long series, consider whether
  some commits should be squashed together or addressed in a separate topic.
- Make sure your commit messages are in the proper format.
- Where appropriate, please update the documentation.
- Ensure that each changed file has the correct copyright and license
  information.
    - Files that entirely consist of contributions to this project should have a
      copyright notice and BSD-3-Clause SPDX license identifier of the form as
      shown in [license.md](./license.md)
    - Files that contain changes to imported Third Party IP files should retain
      their original copyright and license notices.
- If you are submitting new files that you intend to be the technical
  sub-maintainer for (for example, a new platform port), then also update the
  [maintainers](./maintainers.md) file.
- For topics with multiple commits, it is recommended that you make all the
  documentation changes (and nothing else) in the last commit of the series.
- Please test your changes. As a minimum, ensure you can do an AP boot.

Submitting Changes
------------------

- Ensure forking the repository.
  This requires the fork permission on the account, which can be obtained by
  following: https://gitlab.arm.com/documentation/contributions.
- Ensure that each commit in the series has at least one `Signed-off-by:` line,
  using your real name and email address. The names in the `Signed-off-by:`
  and `Author:` lines must match. If anyone else contributes to the commit,
  they must also add their own `Signed-off-by:` line. By adding this line the
  contributor certifies the contribution is made under the terms of the
  [Developer Certificate of Origin (DCO)](./dco.txt).
- Push your local changes to a new branch in your **fork**
  (this might require setting up your SSH public key in your profile).
  See [SSH](https://docs.gitlab.com/ee/user/ssh.html#add-an-ssh-key-to-your-gitlab-account)
- Submit a [merge request](https://docs.gitlab.com/ee/user/ssh.html#add-an-ssh-key-to-your-gitlab-account)
  from your **fork** to the [SCP-firmware](https://gitlab.arm.com/firmware/SCP-firmware)
  _:warning: Please ensure you chose
  [SCP-firmware](https://gitlab.arm.com/firmware/SCP-firmware) `main` as the target branch._
    - The changes in the merge request (MR) will then undergo further review and
      testing by the maintainers. Any review comments will be made as
      comments on the MR.
      This may require you to do some rework.
    - When the changes are accepted, the maintainers will
      integrate them.
    - Typically, the maintainers will merge (fast-forward) the MR into the
      `main` branch within the Gitlab UI by rebasing and then merging.
    - Please avoid creating merge commits in the MR itself.
    - If the MR is not based on a recent commit, the maintainers may rebase it
      onto the `main` branch first, or ask you to do this.
    - If the MR cannot be automatically merged, the maintainers will ask you
      to rebase it onto the `main` branch.
    - If after merging the maintainers find any issues, they may remove the
      commits and ask you to create a new merge request to
      resolve the problems.

Followthrough
-------------

- From time to time new features may be added and older features/functionality
may deprecated. Code owners would be required to modify their code to support
such new requirements within a reasonable time. Failure to do so may result
in the code being deprecated and no longer maintained as a part of the `main`
branch.

- Any regressions must be fixed as soon as possible. If you are unwilling or
unable to fix the regression, (and nobody else does it for you), your commit
might be removed.

- The author of the commit is responsible for fixing any bugs in a timely
manner.  Failure to address issues/bugs may result in the causal commit
being removed.

--------------
*Copyright (c) 2018-2024, Arm Limited and Contributors. All rights reserved.*
