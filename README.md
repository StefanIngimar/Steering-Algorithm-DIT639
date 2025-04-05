# 2025-group-04

This repository contains our group's solution for the PrimeChecker project in the DIT638 course at Chalmers. 
It demonstrates Git-based development using terminal workflows, version control best practices, and C++ programming.

##Pipeline status
![Pipeline status](https://gitlab.example.com/dit638/2025-group-04/badges/5-set-up-ci-cd/pipeline.svg)
The status of this badge reflects the health of the project's CI/CD pipeline. A passing status ensures that the code in the repository is up to date and tested. A red badge indicates that the latest commit has failed the pipeline checks.

## Requirements and Dependencies

To build and run this project, the following dependencies are required:

- A terminal environment (macOS Terminal, Windows PowerShell or Git Bash)
- C++17-compliant compiler:
  - macOS: `clang++` (included with Xcode Command Line Tools)
  - Windows: `g++` (install via MinGW or WSL)
- `make` (required to use the provided Makefile)

## Set up on macOS and Windows

This project uses a terminal-based workflow and requires a C++17 compiler and make. Follow the instructions below to set up your environment.

### Linux

1. Open your terminal.
2. Install development tools using your package manager:

#### Debian/Ubuntu-based:
```bash
sudo apt update
sudo apt install build-essential
```

#### Fedora-based:
```bash
sudo dnf groupinstall "Development Tools"
```

#### Arch-based:
```bash
sudo pacman -S base-devel
```

3. Verify installation:
```bash
g++ --version
make --version
```

You are now ready to build the project using the provided Makefile.

---

### Windows

1. Install **WSL** (e.g., Ubuntu)  
   https://learn.microsoft.com/en-us/windows/wsl/install

2. Open your WSL terminal.

3. Install build tools:

```bash
sudo apt update
sudo apt install build-essential
```

4. Verify installation:

```bash
g++ --version
make --version
```

You can now clone and build the project from within WSL.

---

### macOS
1. Open **Terminal**.
2. Install Xcode Command Line Tools:

```bash
xcode-select --install
```

This installs:
- `clang++` (C++17-compliant compiler)
- `make`
- Developer headers and other essential tools

3. Verify installation:

```bash
clang++ --version
make --version
```

You are now ready to build the project using the provided Makefile.

---

## Way of Working

We follow an agile Scrum methodology. Our work is organized on a shared Trello board where we track tasks as tickets.
Team members pick up tickets from the "Backlog" column, move them to "In Progress" during development, which moves on to "Code Review" upon opening a pull request, 
and finally to "Done" after successful peer review and merge.
Our team uses Git with a feature-branch workflow, peer reviews, and consistent commit message conventions. 

### Adding New Features
1. Pick a ticket from the Trello board (**Backlog** column)
2. Create a branch using the ticket name: `git checkout -b <ticket-name>`. We do not include prefixes like `feature/` or `fix/`; the ticket name is enough.
3. Implement the feature in the `demo/` folder (soon to be`src/` upon actual development)
4. Commit changes using the Angular commit style [(see below)](#commit-messages)
5. Push the branch and open a Pull Request (PR) on GitLab
6. At least one team member must review and approve before merging to `main`

### Bug Fixes
1. Immediately create a ticket on Trello when a bug is identified
2. Create a fix branch using the ticket name:git checkout -b <ticket-name>
3. Apply and test the fix
4. Commit with a descriptive message using the Angular commit style
5. Push and create a Pull Request
6. Peer review required before merging

### Commit Messages
Branch names reflect ticket names from Trello. The type of change (feature, bug fix, docs, etc.) is indicated through the commit message using the Angular commit style.

We follow the Angular commit message style:
`<type>(<scope>): <short description>`

#### Common types:
- `feat`: new feature
- `fix`: bug fix
- `docs`: documentation only
- `style`: formatting (no logic changes)
- `refactor`: code refactoring
- `test`: adding/modifying tests
- `chore`: project maintenance

Read more here: [Angular Commit Message Guidelines](https://gist.github.com/pmutua/7008c22908f89eb8bd21b36e4f92b04f)

### Code Reviews
All Pull Requests must be reviewed by at least one team member. Reviewers check:
- Correctness
- Compilation success
- Style and documentation
- Clear, meaningful commit messages

### Merging Strategy
- All branches must be merged into `main` via Pull Requests (PRs) on GitLab.
- Every PR must be reviewed and approved by at least one teammate.
