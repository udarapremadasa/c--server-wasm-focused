# WebAssembly Node.js Integration Example

This directory contains examples of how to integrate the C++ HTTP server with Node.js using WebAssembly.

## Files

- `server.js` - Node.js server that uses the WebAssembly module
- `package.json` - Node.js dependencies
- `test-client.js` - Simple test client

## Usage

1. Build the WebAssembly module:
```bash
./build.sh --wasm
```

2. Copy the generated WebAssembly files to this directory:
```bash
cp build-wasm/httpserver.js build-wasm/httpserver.wasm examples/nodejs/
```

3. Install Node.js dependencies:
```bash
cd examples/nodejs
npm install
```

4. Run the server:
```bash
node server.js
```

5. Test the server:
```bash
node test-client.js
```
