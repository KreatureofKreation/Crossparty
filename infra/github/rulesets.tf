# SPDX-FileCopyrightText: 2026 KreatureofKreation
# SPDX-License-Identifier: MIT

locals {
  # GitHub's ID for the GitHub Actions app. Requiring checks from this app
  # means another app cannot satisfy them with a status of the same name.
  github_actions_app_id = 15368

  # The job names in .github/workflows/ci.yml and codeql.yml.
  required_checks = [
    "Lint, scan and unit tests",
    "Flatpak build and tests",
    "Analyse c-cpp",
    "Analyse actions",
  ]
}

# main and staging change only through pull requests whose checks have
# passed: no direct pushes, no force pushes, no deletion. Nobody bypasses
# this, including the owner (plan: Workflow).
resource "github_repository_ruleset" "protected_branches" {
  name        = "Protect main and staging"
  repository  = github_repository.crossparty.name
  target      = "branch"
  enforcement = "active"

  conditions {
    ref_name {
      include = ["refs/heads/main", "refs/heads/staging"]
      exclude = []
    }
  }

  rules {
    deletion         = true
    non_fast_forward = true

    # A solo project: no approvals are required, but every change still
    # goes through a reviewable pull request.
    pull_request {
      required_approving_review_count = 0
      allowed_merge_methods           = ["merge", "squash"]
    }

    required_status_checks {
      dynamic "required_check" {
        for_each = local.required_checks
        content {
          context        = required_check.value
          integration_id = local.github_actions_app_id
        }
      }
    }
  }
}

# A release tag always points at the code that was released: once made, a
# v* tag cannot be moved or deleted.
resource "github_repository_ruleset" "release_tags" {
  name        = "Protect release tags"
  repository  = github_repository.crossparty.name
  target      = "tag"
  enforcement = "active"

  conditions {
    ref_name {
      include = ["refs/tags/v*"]
      exclude = []
    }
  }

  rules {
    deletion         = true
    update           = true
    non_fast_forward = true
  }
}
