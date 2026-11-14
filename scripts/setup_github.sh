#!/usr/bin/env bash
# One-time setup of labels and milestones in the GitHub repository.
# Run from the repository root after `gh auth login`. Safe to run again.
set -uo pipefail

label() {
    gh label create "$1" --color "$2" --description "$3" 2>/dev/null \
        && echo "created label: $1" || echo "exists:        $1"
}

label "requirements"    "5319e7" "System or software requirements"
label "design"          "1d76db" "Architecture, detailed design or ADR"
label "change-request"  "d93f0b" "Proposed change to a baselined artifact (ICD, requirements)"
label "icd"             "fbca04" "Communication matrix and interfaces between components (icd/car_gateway.dbc)"
label "ci"              "0e8a16" "Continuous integration and checks"
label "tools"           "c5def5" "Developer tools and code generators"
label "area:core"       "bfdadc" "Scheduler, clock and shared infrastructure"
label "area:comm"       "bfdadc" "Communication layer: bus, I/O threads, codec, E2E"
label "area:managers"   "bfdadc" "Manager base class, unit managers and fault handling"
label "area:sim"        "bfdadc" "Simulated car (test environment)"
label "area:dashboard"  "bfdadc" "Terminal and web dashboard"

milestone() {
    gh api "repos/{owner}/{repo}/milestones" -f title="$1" -f description="$2" >/dev/null 2>&1 \
        && echo "created milestone: $1" || echo "exists:            $1"
}

milestone "v0.1.0" "Phase 1: working skeleton - scheduler, communication, engine, brakes, lighting, terminal view"
milestone "v0.2.0" "Phase 2: full simulation - all units, BIT, fault injection, web dashboard"
milestone "v1.0.0" "Phase 3: real car - Raspberry Pi, OBD-II, GPS and PPS, watchdog"
