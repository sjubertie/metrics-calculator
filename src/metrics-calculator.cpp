#include <iostream>
#include <vector>

#include <llvm/Support/CommandLine.h>

#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>


#include "MetricsCalculatorAction.hpp"


static llvm::cl::OptionCategory myToolCategory("my-tool options");


/**
 *
 */
int main( int argc, char const * argv[] )
{
  // Parse command line parameters.
  clang::tooling::CommonOptionsParser optionsParser( argc, argv, myToolCategory );

  // Create a tool from parameters.
  clang::tooling::ClangTool tool( optionsParser.getCompilations(), optionsParser.getSourcePathList() );

  MyRecursiveASTVisitor::sourcepathlist = optionsParser.getSourcePathList();
  
  // Display compiles commands.
  for( auto & s : optionsParser.getCompilations().getAllCompileCommands() ) {
    for( auto & t : s.CommandLine ) {
      std::cout << t << std::endl;
    }
  }

  
  // Display source files.
  std::cout << "Source files:" << std::endl;
  for( auto const & path : optionsParser.getSourcePathList() ) {
    std::cout << path << std::endl;
  }
  std::cout << std::endl;

  
  // Run the tool !
  int res = tool.run( clang::tooling::newFrontendActionFactory<MetricsCalculatorAction>().get() );

  
  // Operators
  std::cout << "Operators:" << std::endl;
  for( auto const & op : MyRecursiveASTVisitor::operators) {
    std::cout << '(' << op.first << ", " << op.second  << ") ";
  }
  std::cout << std::endl << std::endl;

  
  // Operands
  std::cout << "Operands:" << std::endl;
  for( auto const & op : MyRecursiveASTVisitor::operands) {
    std::cout << '(' << op.first << ", " << op.second  << ") ";
  }
  std::cout << std::endl << std::endl;

  
  // Metrics
  std::cout << "Metrics:" << std::endl;

  unsigned int N1, N2, n1, n2, N, n, E;
  float D, V;
  
  N1 = 0;
  for( auto op : MyRecursiveASTVisitor::operators ) {
    N1 += op.second;
  }

  N2 = 0;
  for( auto op : MyRecursiveASTVisitor::operands ) {
    N2 += op.second;
  }
  
  n1 = MyRecursiveASTVisitor::operators.size();
  n2 = MyRecursiveASTVisitor::operands.size();

  N = N1 + N2;
  n = n1 + n2;
  V = N * std::log2f( n ); //( sizeof( unsigned int ) * 8 - __builtin_clz( n ) );
  D = ( n1 / 2.f ) * ( N2 / (float)n2 );
  E = V * D;
    
  std::cout << "N1=\t" << N1 << std::endl;
  std::cout << "N2=\t" << N2 << std::endl;
  std::cout << "n1=\t" << n1 << std::endl;
  std::cout << "n2=\t" << n2 << std::endl;
  std::cout << "N=\t" << N << std::endl;
  std::cout << "n=\t" << n << std::endl;
  std::cout << "V=\t" << V << std::endl;
  std::cout << "D=\t" << D << std::endl;
  std::cout << "E=\t" << E << std::endl;
  
  return 0;
}
