# 00 — Visão do produto

## Em uma frase

Engine **3D nativa** (Rust) com mapas **brush/displacement**, look **retro
console**, editor embutido **estilo Unity** e gameplay em **Ori** (AOT).

## Problema que resolve

Criar jogos no espírito PS1 / N64 / GameCube com:

- controle artístico (low poly, texturas cruas, fog, post barato);
- edição de nível tipo Quake / Source-lite (faces, brushes, displacements);
- uma linguagem **nossa** (Ori) como C++ de projeto na Unreal;
- sistemas de gameplay reutilizáveis (stealth, horda, IA, veículos depois).

## Princípios

1. **Reading-first** — cena e código legíveis; sem mágica opaca sem dados/código.
2. **Dados ≠ comportamento** — mapas/assets são dados; lógica é Ori (+ Rust na engine).
3. **Uma API de jogo** — tudo que o gameplay toca passa por `game.*` / `ori_engine_*`.
4. **Retro por pipeline**, não por hardware fraco — budgets artísticos + shaders.
5. **Protótipos guiam o v0** — Megabonk-like e Tenchu 2-like definem o corte.
6. **Progressive disclosure** — editor simples no dia 1; tools avançadas depois.
7. **Chunks quando open world** — não heightmap infinito como paradigma de mapa.

## O que somos

| Somos | Não somos |
|-------|-----------|
| Engine + editor para jogos 3D low-poly | Clone de Unreal/Unity |
| Host Rust + gameplay Ori | Engine web (Three.js core) |
| Brushes + displacements | Só landscape heightmap |
| Rapier3D no v1 | Física AAA custom no dia 1 |
| Look PS1/N64/GC configurável | PBR/fotorealismo |
| Irmão de `ori-lang` (só) | Runtime do package `ori-game` (2D/raylib) |

## Personas

| Quem | Quer |
|------|------|
| Autor da engine | Controle total do pipeline e da API |
| Game designer (você) | Mapas rápidos, stealth e hordas, molho visual barato |
| Futuro colaborador | Docs + ADRs + crates claros |

## Critério de sucesso (v0)

- Abrir editor, esculpir um mapa brush, aplicar material retro.
- Rodar personagem com física e câmera 3rd person.
- Script Ori controla update e spawna entidade.
- Um slice **Megabonk** (horda + XP) e um slice **Tenchu** (guarda + visão) demonstráveis.
- UI do editor reconhecível como “família Unity” (docks, hierarchy, inspector).

## Licença

**MIT** + `NOTICE` + política de atribuição: jogos devem referenciar
**Ori Game Engine** nos créditos. Ver [17-licenca-e-atribuicao.md](17-licenca-e-atribuicao.md).

## Plataformas de export (jogos)

| Plataforma | v0/v1 |
|------------|-------|
| **Linux** | Sim |
| **Windows** | Sim |
| macOS | Fora por enquanto |
| Consoles / mobile | Fora |

## Fora de escopo explícito (v0)

- Multiplayer / netcode
- OriScript / VM de script paralela
- Box3D (fica como backend futuro)
- Fluid simulation realista
- Soft-body neve
- GOAP completo no dia 1
- Marketplace de assets
- Mobile-first
- Export macOS (até decisão explícita)
- Dependência do package **`ori-game`** / raylib (cenário A — ADR-0010)

Ver também: [11-prototipos.md](11-prototipos.md), [12-roadmap.md](12-roadmap.md),
[adr/0010-migracao-ori-lang-cenario-a.md](adr/0010-migracao-ori-lang-cenario-a.md).
