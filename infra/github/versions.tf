# SPDX-FileCopyrightText: 2026 KreatureofKreation
# SPDX-License-Identifier: MIT
#
# The repository's GitHub settings as code. Applied by hand from the dev
# container, never from CI. See README.md here and docs/security.md#sec-021.

terraform {
  required_version = ">= 1.8.0"

  required_providers {
    github = {
      source  = "integrations/github"
      version = "6.13.0"
    }
  }

  # The state file records the settings and the IDs of what was created. It is
  # encrypted on this machine before it is written, and plan files with it, so
  # a copied state file is useless without the passphrase. It never goes in
  # the repo (.gitignore). Without the passphrase nothing runs at all.
  encryption {
    key_provider "pbkdf2" "passphrase" {
      passphrase = var.state_passphrase
    }

    method "aes_gcm" "state" {
      keys = key_provider.pbkdf2.passphrase
    }

    state {
      method   = method.aes_gcm.state
      enforced = true
    }

    plan {
      method   = method.aes_gcm.state
      enforced = true
    }
  }
}

# The token comes from the GITHUB_TOKEN environment variable, typed in for
# one session and never saved in a file (README.md here).
provider "github" {
  owner = "KreatureofKreation"
}
