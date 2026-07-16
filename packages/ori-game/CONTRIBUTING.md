# Contributing to ori-game

Thank you for your interest in contributing to ori-game! This document provides guidelines and information for contributors.

## How to Contribute

### Reporting Bugs

1. Check existing issues on GitHub to avoid duplicates
2. Create a new issue with:
   - Clear title describing the bug
   - Steps to reproduce
   - Expected behavior
   - Actual behavior
   - Ori version and platform

### Suggesting Features

1. Check existing issues for similar suggestions
2. Create a new issue with:
   - Clear title describing the feature
   - Use case and motivation
   - Proposed API/interface (if applicable)
   - Any implementation ideas

### Submitting Changes

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes
4. Add tests if applicable
5. Update documentation if needed
6. Commit your changes (`git commit -m 'Add amazing feature'`)
7. Push to the branch (`git push origin feature/amazing-feature`)
8. Open a Pull Request

## Development Setup

### Prerequisites

- Ori compiler (latest version)
- Raylib development libraries
- C compiler (GCC, Clang, or MSVC)

### Building

```bash
# Clone the repository
git clone https://github.com/raillen/ori-game.git
cd ori-game

# Build raylib (if not already installed)
# Follow raylib build instructions for your platform

# Test with Ori compiler
ori check examples/hello_game.orl
```

## Code Style

### Ori Code

- Use 4 spaces for indentation
- Follow Ori 0.3.x naming (`docs/spec/02-lexical.md`):
  - functions / variables / modules: **snake_case** (not camelCase)
  - types / traits / enums: **PascalCase**
  - visibility: **`public`** (not `pub`)
- Add documentation comments for public functions
- Keep functions focused and concise

### File Organization

- Core modules go in `game/`
- Mechanics go in `game/mechanics/`
- Examples go in `examples/`
- Tests go in `tests/`
- Documentation goes in `.oridoc` files

## Documentation

- Add `.oridoc` files for new modules
- Include examples in documentation
- Keep documentation up-to-date with code changes

## Testing

- Add tests for new features
- Ensure existing tests pass
- Test on multiple platforms if possible

## Questions?

Feel free to open an issue for any questions about contributing.
