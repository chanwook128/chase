
// Generated from //wsl$/Ubuntu-20.04/home/lora/chase/repo/src/logics_frontend/grammar\LogicsContracts.g4 by ANTLR 4.9.2

#pragma once


#include "antlr4-runtime.h"




class  LogicsContractsLexer : public antlr4::Lexer {
public:
  enum {
    T__0 = 1, T__1 = 2, T__2 = 3, T__3 = 4, T__4 = 5, T__5 = 6, T__6 = 7, 
    T__7 = 8, T__8 = 9, T__9 = 10, T__10 = 11, T__11 = 12, T__12 = 13, T__13 = 14, 
    T__14 = 15, T__15 = 16, WS = 17, LINE_COMMENT = 18, AND = 19, OR = 20, 
    NOT = 21, IMPLIES = 22, IFF = 23, COLON = 24, ALWAYS = 25, EVENTUALLY = 26, 
    NEXT = 27, UNTIL = 28, RELEASE = 29, EQ = 30, NEQ = 31, LT = 32, LE = 33, 
    GT = 34, GE = 35, PLUS = 36, MINUS = 37, TIMES = 38, DIVIDE = 39, LBRACKET = 40, 
    RBRACKET = 41, LSQUARE = 42, RSQUARE = 43, COMMA = 44, DOT = 45, ID = 46, 
    DECIMAL = 47, NUMBER = 48, ENDST = 49
  };

  explicit LogicsContractsLexer(antlr4::CharStream *input);
  ~LogicsContractsLexer();

  virtual std::string getGrammarFileName() const override;
  virtual const std::vector<std::string>& getRuleNames() const override;

  virtual const std::vector<std::string>& getChannelNames() const override;
  virtual const std::vector<std::string>& getModeNames() const override;
  virtual const std::vector<std::string>& getTokenNames() const override; // deprecated, use vocabulary instead
  virtual antlr4::dfa::Vocabulary& getVocabulary() const override;

  virtual const std::vector<uint16_t> getSerializedATN() const override;
  virtual const antlr4::atn::ATN& getATN() const override;

private:
  static std::vector<antlr4::dfa::DFA> _decisionToDFA;
  static antlr4::atn::PredictionContextCache _sharedContextCache;
  static std::vector<std::string> _ruleNames;
  static std::vector<std::string> _tokenNames;
  static std::vector<std::string> _channelNames;
  static std::vector<std::string> _modeNames;

  static std::vector<std::string> _literalNames;
  static std::vector<std::string> _symbolicNames;
  static antlr4::dfa::Vocabulary _vocabulary;
  static antlr4::atn::ATN _atn;
  static std::vector<uint16_t> _serializedATN;


  // Individual action functions triggered by action() above.

  // Individual semantic predicate functions triggered by sempred() above.

  struct Initializer {
    Initializer();
  };
  static Initializer _init;
};

