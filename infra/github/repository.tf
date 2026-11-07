# SPDX-FileCopyrightText: 2026 KreatureofKreation
# SPDX-License-Identifier: MIT

# The repository already exists, so it is adopted rather than created.
import {
  to = github_repository.crossparty
  id = "Crossparty"
}

import {
  to = github_repository_vulnerability_alerts.crossparty
  id = "Crossparty"
}

import {
  to = github_repository_dependabot_security_updates.crossparty
  id = "Crossparty"
}

resource "github_repository" "crossparty" {
  name        = "Crossparty"
  description = "Unofficial Linux desktop app for Xbox party chat"
  visibility  = "public"

  # Written out to match the repo as it is: left unset, some of these
  # default to off and the first apply would quietly change them.
  has_issues      = true
  has_projects    = true
  has_wiki        = false
  has_discussions = false

  # Feature branches are deleted once their pull request is merged.
  delete_branch_on_merge = true

  # Secret scanning checks every push for known kinds of keys and tokens, and
  # push protection refuses a push that contains one. A second layer behind
  # the pre-commit scan (SEC-004).
  security_and_analysis {
    secret_scanning {
      status = "enabled"
    }
    secret_scanning_push_protection {
      status = "enabled"
    }
  }

  # A mistaken `tofu destroy` must never delete the project.
  archive_on_destroy = true

  lifecycle {
    prevent_destroy = true
  }
}

# Dependabot alerts for known vulnerabilities in dependencies, and pull
# requests that fix them. Version updates are configured in
# .github/dependabot.yml.
resource "github_repository_vulnerability_alerts" "crossparty" {
  repository = github_repository.crossparty.name
  enabled    = true
}

resource "github_repository_dependabot_security_updates" "crossparty" {
  repository = github_repository.crossparty.name
  enabled    = true

  depends_on = [github_repository_vulnerability_alerts.crossparty]
}
