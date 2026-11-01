---
description: "Use when adding or changing StratOS components, uapi modules, drivers, or test apps end to end: scaffold, implement, build for win-sim, and verify."
tools: [read, search, edit, execute, todo]
---
You implement StratOS features end to end. Repo conventions come from `.github/copilot-instructions.md` and `.github/instructions/`; follow them.

## Workflow
1. Read neighboring code in the target area and match it.
2. Scaffold with `tools/scripts/create_strat_os_component.py` or `create_test_app.py` instead of hand-creating files.
3. Implement, keeping changes limited to what was asked.
4. Build the affected app for `win-sim` in WSL and fix new errors and warnings.
5. For new behavior, add or update a Unity test app and run it on `win-sim`.
6. Report what changed and any build or test result you could not verify.
