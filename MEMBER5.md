# Member 5 — Testing, Documentation and Integration

## 1. Responsibilities

Member 5 was responsible for testing the integrated MiniOS project, maintaining project documentation, preparing automated verification, and checking repository readiness.

## 2. Testing Implementation

Created `tests/test_m5.sh` to perform automated integration testing of MiniOS.

The script checks:

- Successful compilation of MiniOS.
- Shell startup and help command.
- System information display.
- Linux process listing.
- Background job submission and completion.
- Worker status display.
- Race-condition demonstration.
- Graceful shell and runtime shutdown.

## 3. Makefile Integration

Updated the Makefile so that the Member 5 integration test runs as part of the complete test suite.

Command:

```bash
make test
```

The integration test can also be executed independently:

```bash
bash tests/test_m5.sh
```

## 4. Documentation

Updated `README.md` to include the integration test and its execution instructions.

The documentation describes the project features, architecture, requirements, build process, available shell commands, examples, testing and benchmarking.

## 5. Continuous Integration

Created the GitHub Actions workflow:

`.github/workflows/minios-tests.yml`

The workflow uses an Ubuntu environment to:

1. Check out the repository.
2. Clean previous build outputs.
3. Build MiniOS and its example programs.
4. Execute the automated test suite.

## 6. Verification Results

The GitHub Actions workflow completed successfully.

- MiniOS build: Passed
- Example program build: Passed
- Automated test suite: Passed
- Member 5 integration test: Passed

## 7. Conclusion

The Member 5 contribution provides automated integration testing and continuous verification for MiniOS. The updated documentation and GitHub Actions workflow make the project easier to build, test, maintain and verify.
