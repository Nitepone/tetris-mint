# Developing

## Formatting

The code in this repository is automatically formatted with clang-format.

If you want to automatically check your code before each commit, run:

```bash
mkdir -p .git/hooks
cp docs/pre-commit .git/hooks
```

# Appendix

## Clang-Format

- clang-format version 10 added a number of nice features, such as a check
  accessible via the `--dry-run` flag. This is available on Debian testing and
  the latest Ubuntu release, but not older releases (though it can be installed
  with a little finagling). See [commit 6e1f7d6][6e1f7d6].
- For the future, we may also want to look at some of the nice scripts that
  LLVM maintains in their clang/tools directory.

[6e1f7d6]: https://github.com/llvm/llvm-project/commit/6a1f7d6c9ff8228328d0e65b8678a9c6dff49837
