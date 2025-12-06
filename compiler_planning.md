Compiler Phases

- Build GUI skeleton with some hardcoded (but realistic) data to test (source code -> tokens -> AST -> IR) interactivity chain

- preprocess_text (leverage existing lexer code from fmtcpp), integrate into GUI

- preprocessed_text_to_tokens (cl /P), integrate into GUI

- Loop of (tokens_to_AST + AST_to_IR + execute_IR_inst)
  - Integrate one language feature at a time
    - tokens -> AST nodes
    - AST nodes -> custom IR
    - Interpret and execute custom IR
    - AST nodes -> standard IR
    - Standard IR -> OBJ
    - Custom IR -> OBJ
    - Each feature contributes to growing test suite which is managed in GUI
  - Build out features needed to compile and execute cJSON programs, from easiest to hardest
  - Then build out features needed to compile and execute Lua programs, from easiest to hardest
