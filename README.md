# Tiny Compiler

A simple compiler for the TINY language, written in C++. This compiler can parse and execute TINY language programs.

## Introduction

This project implements a compiler for the TINY language, a minimal programming language designed for educational purposes. The compiler is written in C++ and includes a parser, analyzer, and interpreter for TINY language programs.

## Features

- **Lexer and Parser:** Tokenizes and parses TINY language programs.
- **Syntax Tree:** Builds a syntax tree from parsed programs.
- **Symbol Table:** Manages variables and their memory locations.
- **Semantic Analysis:** Performs semantic analysis to check for type errors.
- **Interpreter:** Executes TINY language programs.

## Getting Started

## Program Structure

The TINY language programs should adhere to the specified grammar. The main components include:

- **Lexer and Parser:** Tokenizes the input and constructs a syntax tree.
- **Analyzer:** Performs semantic analysis, builds the symbol table, and fills in type information.
- **Interpreter:** Executes the TINY language program.

## Analyzer

The analyzer is responsible for filling the symbol table, performing semantic analysis, and checking for type errors. It also calculates the memory locations for variables.
