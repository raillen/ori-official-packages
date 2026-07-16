# {{APP_NAME}}

Notes skeleton (ori-templates SSR). Scaffolded by `tools/ori-web new … --template=notes`.

## Run

```bash
ori get .
ori run main.orl
```

## Clean code

- Handlers stay small; no HTML string building for pages.
- User input only via forms + CSRF; templates escape by default.
- See `docs/CLEAN-CODE.md` in ori-web-framework.
