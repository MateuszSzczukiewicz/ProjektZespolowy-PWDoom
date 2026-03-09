# Contributing to PWDoom

## Git Flow

```
main          <- stable releases (tagged: v0.1.0, v0.2.0, ...)
  |
develop       <- integration branch (all features merge here)
  |
feature/xxx   <- feature branches (from develop)
release/x.x.x <- release prep (from develop, merges to main + develop)
hotfix/xxx    <- production fixes (from main, merges to main + develop)
```

## Branch Naming

| Type | Pattern | Example |
|------|---------|---------|
| Feature | `feature/short-description` | `feature/raycasting-engine` |
| Bugfix | `fix/short-description` | `fix/player-collision` |
| Release | `release/x.y.z` | `release/0.2.0` |
| Hotfix | `hotfix/short-description` | `hotfix/crash-on-exit` |

## Workflow

1. Create feature branch from `develop`:
   ```bash
   git checkout develop
   git pull
   git checkout -b feature/my-feature
   ```

2. Work, commit (conventional commits):
   ```bash
   git commit -m "feat: add player movement"
   git commit -m "fix: wall collision detection"
   git commit -m "refactor: extract map loader"
   ```

3. Push and create PR to `develop`:
   ```bash
   git push -u origin feature/my-feature
   gh pr create --base develop
   ```

4. After review + CI pass -> merge to `develop`

5. Release:
   ```bash
   git checkout develop
   git checkout -b release/0.2.0
   # bump version in CMakeLists.txt, test, fix
   # merge to main + develop, tag
   ```

## Commit Messages (Conventional Commits)

```
feat:     new feature
fix:      bug fix
refactor: code change (no new feature, no fix)
docs:     documentation
ci:       CI/CD changes
test:     tests
chore:    maintenance
```

## Versioning (Semantic Versioning)

`MAJOR.MINOR.PATCH` (set in CMakeLists.txt)

- **MAJOR**: breaking changes / big milestones
- **MINOR**: new features (raycasting, enemies, weapons)
- **PATCH**: bug fixes

Current: `0.1.0` (initial scaffold)
