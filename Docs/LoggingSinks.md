# DraconicEngine Logging Sinks

The logging module in DraconicEngine utilizes an asynchronous sink architecture. Sinks are responsible for formatting and routing `LogMessage` structs to their final destination. 

Currently, the engine provides three native sinks, all implementing the `ILogSink` interface:

## 1. `ConsoleSink`
Outputs standard formatted logs directly to `stdout`.
* **Usage:** Best for local debugging and immediate developer feedback inside IDE terminals.
* **Performance:** Uses high-speed async queuing, but `stdout` blocking can occur depending on the OS terminal implementation.

## 2. `FileSink`
Appends incoming log messages to a specified flat file path.
* **Usage:** Standard, long-running application logs where size constraints aren't an immediate concern.
* **Internal:** Implemented using the PIMPL idiom to avoid leaking `<fstream>` into the module interface.

## 3. `RollingFileSink`
Behaves like `FileSink`, but intelligently rolls the log to a new file when a predefined size limit (`m_max_file_size`) is hit.
* **Usage:** Production builds and dedicated servers where you need to prevent log files from infinitely ballooning and consuming disk space.
* **Formatting:** Appends a timestamp (`%Y%m%d_%H%M%S`) and an atomic sequence number to the new filename.
