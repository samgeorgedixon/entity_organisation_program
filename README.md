# Entity Organisation Program

## Overview

This is a program that will organise entities into many cells composed into different zones.

It can be used heavily in areas involving organsising groups of people of a range of sizes based of conditions and zoning.

For instance it can be used for creating seating plans of people in an array of rooms or timetables for large arragements of people in a work place, school or event.

## Installation

There are [pre-built binaries](https://github.com/samgeorgedixon/entity_organisation_program/releases) (Windows_x86-64)

Otherwise use premake to build from source.

## Usage

This is based around "Excel XML files" that have an aray of sheets that you can edit with all the necessary configuration and then run it through the program which outputs into the same file.

It is recomended to start from a template.

### Configuration Sheets
---
- District:

    A District defines the arragement of cells, which cells can be occupied and which if any zone it belongs too.

- Entities:

    Entities are like different people or objects that can occupy one or more cells and have identifiers that can be used to condition it against other entities or zones.

- Identifiers:

    Identifiers are different ways to diferentiate entities by for instance an "ID" or "Name" identifier.

- Iterations:

    Iterations are different variations of the configuration that can have certain cells or zones disabled or carried on from the last iteration.

- Zones:

    Zones are groups of cells that can restrict certain entities with certain identifiers from being within the zone.
