# SPDX-FileCopyrightText: 2026 KreatureofKreation
# SPDX-License-Identifier: MIT

# Two environments for the signed Flatpak repository (SEC-025), both open only
# to jobs running for a v* tag. Only the owner can push tags, the tag
# ruleset stops them moving, and the release workflow refuses a tag that is
# not on main, so nothing from a branch or a pull request can reach the
# signing key or change what users install.

# Holds the FLATPAK_SIGNING_KEY secret. The secret itself is added by hand in
# the web interface, so it never passes through OpenTofu or its state
# (README.md here).
resource "github_repository_environment" "release_signing" {
  repository  = github_repository.crossparty.name
  environment = "release-signing"

  deployment_branch_policy {
    protected_branches     = false
    custom_branch_policies = true
  }
}

resource "github_repository_environment_deployment_policy" "release_signing_tags" {
  repository  = github_repository.crossparty.name
  environment = github_repository_environment.release_signing.environment
  tag_pattern = "v*"
}

# GitHub makes this one when Pages is set to deploy from a workflow, open to
# the default branch. Here it is changed to v* tags only.
resource "github_repository_environment" "github_pages" {
  repository  = github_repository.crossparty.name
  environment = "github-pages"

  deployment_branch_policy {
    protected_branches     = false
    custom_branch_policies = true
  }

  depends_on = [github_repository_pages.crossparty]
}

resource "github_repository_environment_deployment_policy" "github_pages_tags" {
  repository  = github_repository.crossparty.name
  environment = github_repository_environment.github_pages.environment
  tag_pattern = "v*"
}
