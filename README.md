# Entity Organisation Program

## Overview

The Entity Organisation Program can be used to organise a variety of things like people into into a range of different things like seats.

It can be used heavily in areas involving organising groups of people of a range of sizes based of conditions and zoning.

For instance it can be used for creating seating plans of people in an array of rooms or timetables for large arrangements of people in a work place, school or event.

## Installation

There is a windows_x86-64 installer under [releases](https://github.com/samgeorgedixon/entity_organisation_program/releases).

To build from source on windows you can use Premake and replace target.

```shell
dep\premake\premake5.exe <target>
```

Only windows_x86-64 is currently supported at this moment.

## Usage

It is all based around a spreadsheet that you configure based on the type of lua config which defines how the spreadsheet is interpreted and ran through the program into another spreadsheet with the evaluated results inside.

You can start with a lua config template and a spreadsheet example under releases and then read the [documentation](https://samgeorgedixon.github.io/entity_organisation_program) on how to configure these to fit your needs.
