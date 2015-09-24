#ifndef METRICSCALCULATORASTCONSUMER_HPP
#define METRICSCALCULATORASTCONSUMER_HPP


#include <iostream>

#include <clang/AST/ASTConsumer.h>
#include <clang/AST/ASTContext.h>

#include "MyRecursiveASTVisitor.hpp"


class MetricsCalculatorASTConsumer : public clang::ASTConsumer
{

private:

  MyRecursiveASTVisitor myrecvisitor;

public:

  MetricsCalculatorASTConsumer( clang::ASTContext * p_astContext )
    : myrecvisitor( MyRecursiveASTVisitor( p_astContext ) )
  {
  }

  /**
   * Method called only when the entire file is parsed.
   */
    void HandleTranslationUnit( clang::ASTContext & context ) override
    {
      // Recursively visit the AST.
      myrecvisitor.TraverseDecl( context.getTranslationUnitDecl() );
    }

};


#endif
