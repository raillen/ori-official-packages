# web_framework (ori-web-framework)

**Symbolic meta-package** on [OriLamp](https://github.com/raillen/ori-lamp).

It does **not** bundle framework source. The tarball is a tiny manifest + stub;
`ori install` recursively fetches real lamps from the same registry.

## Core stack (this package)

```bash
export ORI_REGISTRY=https://<your-orilamp-host>/registry
ori install web_framework@0.1.0
```

| Pulled | Role |
|--------|------|
| `web@0.1.0` | HTTP, session, middleware |
| `templates@0.1.0` | HTML templates |
| `web_app@0.1.0` | App helpers / generators |

## Optional lamps (not pulled by default)

| Package | When |
|---------|------|
| `web_auth@0.1.0` | TOTP 2FA |
| `web_session_sqlite@0.1.0` | SQLite session store (also pulls `sqlite`) |

```bash
ori install web_auth@0.1.0
ori install web_session_sqlite@0.1.0
```

## App dependency

```toml
[dependencies]
web_framework = "0.1.0"
# optional:
# web_auth = "0.1.0"
# web_session_sqlite = "0.1.0"
```

In source, import the concrete packages (`import web`, etc.).

## Monorepo

Full trees and demos:
[ori-web-framework](https://github.com/raillen/ori-web-framework).
