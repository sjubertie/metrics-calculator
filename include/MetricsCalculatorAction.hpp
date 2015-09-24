#ifndef METRICSCALCULATORACTION_HPP
#define METRICSCALCULATORACTION_HPP


#include <clang/Frontend/FrontendAction.h>
#include <clang/Frontend/CompilerInstance.h>

#include "MetricsCalculatorASTConsumer.hpp"


class MetricsCalculatorAction : public clang::ASTFrontendAction
{
public:

  std::unique_ptr< clang::ASTConsumer > CreateASTConsumer(clang::CompilerInstance & compiler, llvm::StringRef inFile )
  {
    return std::unique_ptr< clang::ASTConsumer >( new MetricsCalculatorASTConsumer( &compiler.getASTContext() ) );
  }

};




#endif
