# EUI - Euclid UI Library

## Introduction

EUI is a simple UI library for the Euclid engine. Features include:

- UI primitives like containers, text, button
- Auto-calculated layouts
- CSS-like styling
- Input handling

## Architecture

The UI library is built on top of the engine's rendering system, raylib.

Each ui needs an `EUI_Context`. This context is responsible for rendering and handling input.
