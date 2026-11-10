# Requirements

The project's requirements are managed with [StrictDoc](https://github.com/strictdoc-project/strictdoc), an open-source requirements tool. They live in this folder as plain-text `.sdoc` files, next to the code they describe.

## Files

| File | Contents |
| --- | --- |
| `system.sdoc` | System requirements (`SYS_*`): what the system does, seen from outside as a black box |
| *(planned)* one `.sdoc` per component | Software requirements (`SW_*`), each linked to its parent `SYS_*` requirement with a `Parent` relation |

## Install

Ubuntu 24.04 does not allow `pip install` into the system Python, so install StrictDoc with `pipx`:

```bash
sudo apt install -y pipx
pipx ensurepath        # then open a new terminal
pipx install strictdoc
```

## Use

Run these from the repository root.

Browse and edit the requirements in the web interface (open the address it prints):

```bash
strictdoc server .
```

Export the requirements as a static HTML site:

```bash
strictdoc export . --output-dir build/strictdoc
```

Export to ReqIF, the format used to exchange requirements between tools such as DOORS, Polarion and Codebeamer:

```bash
strictdoc export . --formats=reqif-sdoc --output-dir build/reqif
```

## Workflow

- Every requirement has a `STATUS`: `Draft` → `Reviewed` (both developers reviewed it) → `Approved` (both agree).
- Any change to a requirement goes through a pull request with a reason, and needs approval from the other developer.
- A requirement ID never changes and is never reused, even if the requirement is deleted.
- A baseline of the requirements is a Git tag: each release tag records exactly which requirements that version meets.
