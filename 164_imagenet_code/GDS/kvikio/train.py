import cupy
import kvikio

def main(path):
    a = cupy.arange(100)
    f = kvikio.CuFile(path, "w")
    # Write whole array to file
    f.write(a)
    f.close()

    b = cupy.empty_like(a)
    f = kvikio.CuFile(path, "r")
    # Read whole array from file
    f.read(b)
    assert all(a == b)
    f.close()

    # Use contexmanager
    c = cupy.empty_like(a)
    with kvikio.CuFile(path, "r") as f:
        f.read(c)
    assert all(a == c)

    # Non-blocking read
    d = cupy.empty_like(a)
    with kvikio.CuFile(path, "r") as f:
        future1 = f.pread(d[:50])
        future2 = f.pread(d[50:], file_offset=d[:50].nbytes)
        # Note: must wait for futures before exiting block
        # at which point the file is closed.
        future1.get()  # Wait for first read
        future2.get()  # Wait for second read
    assert all(a == d)


if __name__ == "__main__":
    main("/tmp/kvikio-hello-world-file")