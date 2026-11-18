# Development environment setup

Everything needed to build, test and contribute to this repository, from a Windows laptop to green tests. Follow the steps in order. If a step does not work as written, fix this document in your first pull request.

## The one rule

**Everything happens inside Linux. Windows is only the window.**

- The repository lives in your Linux home folder (`~/central-vehicle-computer`). Never edit its files through Windows File Explorer.
- Run every command in the Ubuntu terminal. If your prompt starts with `PS`, you are in Windows PowerShell: switch to Ubuntu.
- VS Code must be connected to WSL: the bottom-left corner shows **WSL: Ubuntu**.

## 1. WSL and Ubuntu 24.04

In **PowerShell as administrator**:

```powershell
wsl --install -d Ubuntu-24.04
```

Restart the computer, open **Ubuntu** from the Start menu, and choose a Linux user name and password. Then update the system:

```bash
sudo apt update && sudo apt upgrade -y
```

## 2. Base tools

```bash
sudo apt install -y build-essential cmake git can-utils pipx unzip
pipx ensurepath
```

Close the terminal and open a new one, so that `~/.local/bin` is on your `PATH`.

## 3. GitHub CLI (official repository)

Do not use the `gh` from Ubuntu's own archive: versions 2.45 and 2.46 are broken against the current GitHub API.

```bash
(type -p wget >/dev/null || (sudo apt update && sudo apt install wget -y)) \
  && sudo mkdir -p -m 755 /etc/apt/keyrings \
  && out=$(mktemp) && wget -nv -O$out https://cli.github.com/packages/githubcli-archive-keyring.gpg \
  && cat $out | sudo tee /etc/apt/keyrings/githubcli-archive-keyring.gpg > /dev/null \
  && sudo chmod go+r /etc/apt/keyrings/githubcli-archive-keyring.gpg \
  && echo "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/githubcli-archive-keyring.gpg] https://cli.github.com/packages stable main" | sudo tee /etc/apt/sources.list.d/github-cli.list > /dev/null \
  && sudo apt update \
  && sudo apt install gh -y
gh --version
```

Log in, and choose: **GitHub.com**, **HTTPS**, **Yes** (authenticate Git), **Login with a web browser**. Open the URL it prints in your Windows browser and enter the code.

```bash
gh auth login
```

## 4. Git settings

```bash
git config --global user.name "Your Name"
git config --global user.email "you@example.com"
git config --global core.pager "less -FRX"
git config --global fetch.prune true
```

- `core.pager "less -FRX"`: short output is printed directly; long output opens a pager, which you leave with **q** (not Ctrl+Z).
- `fetch.prune true`: every fetch and pull removes references to branches deleted on GitHub.

## 5. Clone the repository

```bash
gh repo clone levihorvitz/central-vehicle-computer ~/central-vehicle-computer
cd ~/central-vehicle-computer
```

## 6. Python tools

```bash
pipx install strictdoc
pipx install cantools
```

- `strictdoc` manages the requirements (`docs/requirements/`).
- `cantools` checks and displays the communication matrix (`icd/central_vehicle_computer.dbc`).

## 7. Build and test

```bash
cmake -S . -B build
cmake --build build -j
ctest --test-dir build --output-on-failure
```

The first `cmake` downloads GoogleTest. The last line must end with `100% tests passed`.

## 8. Virtual CAN

The simulated car runs on a virtual CAN bus, `vcan0`:

```bash
sudo modprobe vcan
sudo ip link add dev vcan0 type vcan
sudo ip link set up vcan0
```

Check it with two terminals: run `candump vcan0` in the first and `cansend vcan0 123#DEADBEEF` in the second. The frame must appear in the first terminal.

`vcan0` disappears on every restart. If `modprobe vcan` fails, the kernel has no virtual CAN module; the unit tests do not need it, so continue and raise it with the team.

## 9. VS Code

1. Install VS Code on **Windows**, and its **WSL** extension (Microsoft).
2. From the Ubuntu terminal: `cd ~/central-vehicle-computer && code .`
3. Check the bottom-left corner shows **WSL: Ubuntu**.
4. In the Extensions view, install these **in WSL** (button **Install in WSL: Ubuntu**):
   - **C/C++** (Microsoft)
   - **CMake Tools** (Microsoft) — select the **GCC 13** kit when asked
   - **Git Graph**
   - **Markdown Preview Mermaid Support**
5. Settings: enable **Files: Trim Trailing Whitespace**.

If you use Claude Code, read `CLAUDE.md` first: it never commits, pushes or opens pull requests in this repository.

## 10. Everyday commands

| Task | Command |
| --- | --- |
| Build and run all tests | `cmake --build build -j && ctest --test-dir build --output-on-failure` |
| Browse the requirements | `strictdoc server .`, then open http://127.0.0.1:5111 |
| Show the communication matrix | `cantools dump icd/central_vehicle_computer.dbc` |
| Run the scheduler demo | `./build/apps/scheduler_demo 5` |
| See local and GitHub branches | `git branch -vv` |

## 11. How we work

- Every change goes through a branch and a pull request; `main` is protected.
- Pull requests need one approval and green CI, and are merged with **Squash and merge**.
- Work is tracked as GitHub issues on the project board; changes to the ICD or approved requirements start as a change request.
- Read before your first change: `docs/coding-guidelines.md`, `docs/architecture.md`, `docs/adr/`, `docs/requirements/README.md`, `docs/icd/README.md`.

After a pull request is merged:

```bash
git switch main && git pull
git branch -D <branch>
```

## Troubleshooting

| Symptom | Cause and fix |
| --- | --- |
| Prompt starts with `PS` | You are in PowerShell. Open the Ubuntu terminal, or reopen VS Code in WSL (`WSL: Reopen Folder in WSL`). |
| Files named `...:Zone.Identifier` appear | Files were copied from Windows through File Explorer. Delete them, and move downloads with `mv /mnt/c/Users/<you>/Downloads/<file> .` instead. |
| `Permission denied` on a script | Run `chmod +x <script>` once. |
| `[1]+ Stopped` after a git command | You left the pager with Ctrl+Z. Use **q**; clean up with `jobs` and `kill %1`. |
| `gh` prints a GraphQL "Projects (classic)" error | Old `gh` from Ubuntu's archive. Install it from the official repository (step 3). |
| VS Code shows errors but the build passes | IntelliSense is out of date: run `Developer: Reload Window`. |
