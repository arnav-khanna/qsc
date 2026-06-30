# Security Policy

QSC is an experimental compressor and should not yet be used to extract untrusted archives in security-sensitive environments.

## Supported Versions

Only the latest `main` branch is supported during the research-prototype phase.

## Reporting a Vulnerability

Please report vulnerabilities through GitHub Security Advisories if enabled for the repository, or by opening an issue with enough detail to reproduce the problem if the report is not sensitive.

Include:

- operating system and compiler;
- command line used;
- input archive or a minimal reproducer;
- expected behavior;
- actual behavior.

## Known Hardening Gaps

- Archive extraction path handling needs stronger traversal protection.
- Malformed archive validation is incomplete.
- There is no fuzzing corpus checked into the repository yet.
- Chunk and file checksums are not implemented.
