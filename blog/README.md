# Building Linux CI/CD for Embedded Systems: A Human-AI Collaboration Journey

A 4-part technical blog series documenting the development of a comprehensive Linux-based CI/CD pipeline for embedded systems, featuring STM32 microcontrollers, Hardware-in-the-Loop testing, and collaborative AI-assisted development workflows.

## Series Overview

This series examines the systematic development of a production CI/CD pipeline for STM32-based embedded systems. The project demonstrates how human domain expertise can effectively guide AI assistance to overcome complex technical challenges in embedded development, debugging, and test automation.

The collaboration involved transforming a working Arduino IDE based development environment into an automated testing and deployment system through careful incremental development, with the human engineer providing architectural direction, tool selection, debugging guidance, and working reference implementations.

**Target Audience**: Early career embedded engineers with Arduino/STM32 experience who want to understand modern CI/CD practices for embedded systems and effective patterns for AI-assisted development.

## Blog Posts

### Part 1: Architecture and Tool Selection
**File**: `01-architecture-tool-selection.md`
**Focus**: Project requirements, tool selection rationale (use of arduino-cli, J-Link vs ST-Link), and CI/CD architecture decisions

### Part 2: Build Automation and HIL Framework
**File**: `02-build-automation-hil.md`
**Focus**: Developing automated build workflows, environment validation, and reliable Hardware-in-the-Loop execution

### Part 3: Storage Systems Implementation
**File**: `03-storage-systems.md`
**Focus**: SDFS development challenges, debugging methodologies, and unified storage testing framework

### Part 4: Collaborative Development Patterns
**File**: `04-collaborative-development.md`
**Focus**: Effective human-AI collaboration patterns, domain expertise guidance, and debugging strategies

## Key Technologies Covered

- **Hardware**: STM32F411 microcontrollers (Nucleo F411RE, BlackPill F411CE)
- **Debug Tools**: J-Link (selected over ST-Link), SEGGER RTT, J-Run automation
- **Storage Systems**: LittleFS (SPI flash), SDFS (custom SD card filesystem with FatFs backend)
- **Testing**: AUnit framework integration, Hardware-in-the-Loop validation
- **Build Automation**: Arduino CLI workflows, environment validation, device auto-detection
- **Development Methodology**: Human-directed AI assistance, incremental development approach

## Development Context

This series documents the actual development history of the Arduino_dev repository from September 2025. The human engineer provided:

- **Working Foundation**: Functional Arduino core, RTT implementation, and LittleFS library
- **Architectural Direction**: Tool selection (J-Link over ST-Link), dual debug approach (RTT for CI, Serial for IDE)
- **Debugging Guidance**: Working reference implementations, expected output comparisons
- **Hardware Resources**: Physical STM32 boards for Hardware-in-the-Loop testing
- **Incremental Development**: Step-by-step build automation and testing framework development

## Technical Approach

Each blog post examines:
- **Challenge Definition**: Specific technical problems encountered
- **Solution Development**: How human guidance directed AI implementation
- **Debugging Process**: Methodologies for embedded systems troubleshooting
- **Engineering Insights**: Professional practices for early career engineers

## Reading Guide

- **Part 1**: Tool selection rationale and architecture decisions
- **Part 2**: Focus on CI/CD automation techniques and environment management
- **Part 3**: Deep dive into embedded storage systems and debugging methodologies
- **Part 4**: Development patterns for human-AI collaboration

## Repository Integration

All technical content is based on functional, tested code from the repository. Users can:

1. **Study Working Examples**: Review actual commits and implementation decisions
2. **Understand Tool Selection**: Learn the rationale behind J-Link vs ST-Link choices
3. **Apply CI/CD Patterns**: Adapt the automation scripts to their own embedded projects
4. **Practice Debugging**: Follow the methodologies used to resolve complex SDFS implementation challenges

This series serves as technical documentation for building robust embedded CI/CD systems and effective human-AI collaborative development workflows.