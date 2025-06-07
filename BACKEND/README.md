# Compiler Tools Backend API

A RESTful web API built with C++ Crow framework that provides access to LR0 syntax analyzer and regex automata builder tools.

## Project Structure

```
compiler-tools-backend/
├── src/
│   ├── main.cpp               # Application entry point
│   ├── core/                  # Core compiler functionality
│   │   ├── lr0_parser.h       # LR0 parser interface
│   │   ├── lr0_parser.cpp     # LR0 syntax analyzer implementation
│   │   ├── regex_automata.h   # Regex automata interface
│   │   └── regex_automata.cpp # Regex automata builder implementation
│   └── api/                   # API layer
│       ├── handlers.h         # API handlers interface
│       ├── handlers.cpp       # API endpoint implementations
│       ├── routes.h           # Route definitions interface
│       └── routes.cpp         # Route setup and configuration
├── include/                   # Public header files (if needed)
├── data/                      # Test data and configuration files
│   ├── grammars/             # Sample grammar files
│   └── test_inputs/          # Test input files
├── build/                     # Build directory (generated)
├── CMakeLists.txt            # CMake build configuration
├── package.json              # Project metadata
└── README.md                 # This file
```

## Prerequisites

- C++17 or later
- CMake 3.10+
- Crow C++ framework
- macOS, Linux, or Windows with appropriate compiler

### Installing Crow Framework

**macOS (Homebrew):**
```bash
brew install crowcpp/crow/crow
```

**Ubuntu:**
```bash
sudo apt-get install libcrow-dev
```

**From Source:**
```bash
git clone https://github.com/CrowCpp/Crow.git
cd Crow
mkdir build
cd build
cmake .. -DCROW_BUILD_EXAMPLES=OFF
make -j4
sudo make install
```

## Building the Project

1. Clone or extract the project
2. Create build directory and compile:

```bash
mkdir -p build
cd build
cmake ..
make
```

## Running the Server

```bash
cd build
./compiler_api
```

The server will start on `http://localhost:8080`

## API Endpoints

### Health Check
- **GET** `/api/health`
- Returns server status

### LR0 Syntax Analysis
- **POST** `/api/lr0/parse`
- Parse input using LR0 syntax analyzer
- Body: `{"grammar": "E -> E + T | T\\nT -> (E) | a", "input": "a+a"}`

### Regex Automata Building
- **POST** `/api/regex/build`
- Build NFA and DFA from regular expression
- Body: `{"regex": "a*b+"}`

### Regex String Matching
- **POST** `/api/regex/match`
- Test if input string matches regular expression
- Body: `{"regex": "a*b+", "input": "aaabbb"}`

### Grammar File Upload
- **POST** `/api/grammar/upload`
- Upload grammar file content
- Body: `{"content": "E -> E + T | T\\nT -> (E) | a", "filename": "my_grammar.txt"}`

## Usage Examples

```bash
# Health check
curl -X GET http://localhost:8080/api/health

# LR0 parsing
curl -X POST http://localhost:8080/api/lr0/parse \\
  -H "Content-Type: application/json" \\
  -d '{"grammar": "E -> E + T | T\\nT -> (E) | a", "input": "a+a"}'

# Regex matching
curl -X POST http://localhost:8080/api/regex/match \\
  -H "Content-Type: application/json" \\
  -d '{"regex": "a*b+", "input": "aaabbb"}'
```

## Features

- **Timeout Protection**: All API endpoints have configurable timeout limits
- **CORS Support**: Cross-origin requests are supported
- **Error Handling**: Comprehensive error handling with meaningful messages
- **Request Validation**: Input validation and sanitization
- **Performance Monitoring**: Request timing and logging
- **Modular Architecture**: Clean separation between core logic and API layer

## Architecture

### Core Components
- **LR0Parser**: Handles LR0 syntax analysis and grammar processing
- **RegexAutomata**: Manages regex pattern compilation and string matching

### API Layer
- **APIHandlers**: Process HTTP requests and format responses
- **APIRoutes**: Configure URL routing and CORS handling

### Configuration
- Server timeout: 30 seconds
- Maximum regex length: 1000 characters
- Maximum input string length: 10,000 characters
- Concurrent threads: 4

## Development

### Adding New Endpoints
1. Add handler function to `src/api/handlers.cpp`
2. Add route definition to `src/api/routes.cpp`
3. Update API documentation

### Extending Core Functionality
1. Add new functions to appropriate core module (`lr0_parser.cpp` or `regex_automata.cpp`)
2. Update corresponding header files
3. Add API interface in handlers

## License

[Add license information here]

## Contributing

[Add contribution guidelines here]
