# Configuration

Here is how you can configure your Entity Organisation Program's configuration file.

The configuration file is an "Excel Workbook (*.xlsx)" with each sheet corresponding to another step in the configuration.

To start you can download the template from the [releases](https://github.com/samgeorgedixon/entity_organisation_program/releases) page or use an example.

## District Sheet

The district sheet outlines the place you are organising (like a building), all the zones inside of it (like rooms) and which cells are part of each one (like seats).

It starts from the 2nd row with the number in each cell corresponding to the zone it is part of. If a cell is blank then it is excluded from the district.

Make sure to mark the bounds of the district with a '-' in the cell at the bottom left of the district and top right like in the template.

## Entities Sheet

The entities sheet is a list of all the entities (like a person or object) which can have identifier values and conditions.

Each entity first has a count which is the amount of them there are (like 1 for a person).

They also have cell, "(x, y)", and zone, "index", conditions which outline which they are not aloud to be in.

Then for each identifier in the identifiers sheet they have a value and conditions which are values they cannot be with (what this condition means is decided in the identifiers sheet).

Make sure to mark the end of the list with a '-' in the cell at the bottom left.

## Identifiers Sheet

The identifiers sheet is a list of all the identifiers (like an id or name) that can be used to differentiate and condition entities.

Each identifier has a name which matches to the one used in the entities sheet.

Also they have an iteration count which is the amount of times the entity can be part of a zone collapsed into the identifier before the entities value is removed (0 means unlimited here).

They also have relative cell, "(x, y)", and zone, "relative index", conditions that define where the entities conditions can't be (like the condition cant be in a cell next to them or the same zone).

Make sure to mark the end of the list with a '-' in the cell at the bottom left.

## Iterations Sheet

The iterations sheet is a list of all the iterations which are different evaluations of the configuration with alterations (like different periods of time).

Every iteration is evaluated and outputted as a sheet with the given name and the hide option allows you to exclude it from being shown in the output, this is set using a 'T'.

Also you can disable cells, "(x, y)", zones, "index", or entities with an identifier value, "(identifier, value)".

As well you can carry, from certain iterations, cells, "[iteration, (x, y)]", zones, "[iteration, index]", or entities with an identifier value, "[iteration, (identifier, value)]".

They also allow you to disable zones from collapsing into certain identifiers in this iteration, "(identifier, value)".

Make sure to mark the end of the list with a '-' in the cell at the bottom left.

## Zones

The zones sheet is a list of all the zones which are groups of different cells and can be used to restrict or allow certain entities from being there
Zones are groups of cells that can restrict certain entities with certain identifiers from being within the zone.
