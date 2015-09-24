#ifndef MyRecursiveASTVisitor_HPP
#define MyRecursiveASTVisitor_HPP


#include <iostream>
#include <map>
#include <vector>

#include <clang/Basic/SourceManager.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/Stmt.h>


using Map = std::map< std::string, unsigned int >;


class MyRecursiveASTVisitor : public clang::RecursiveASTVisitor< MyRecursiveASTVisitor >
{

public:

  static Map operators;
  static Map operands;
  static std::vector< std::string > sourcepathlist;

private:

  clang::ASTContext * p_astContext;


public:

  MyRecursiveASTVisitor( clang::ASTContext * p_astContext )
    : p_astContext(p_astContext)
  {}

  
  /**
   * Overide the TraverseDecl method to avoid traversing declarations from included files.
   */
  bool TraverseDecl( clang::Decl * p_decl )
  {
    bool isdeclinsource = clang::isa< clang::TranslationUnitDecl >( p_decl ) || p_astContext->getSourceManager().isInMainFile( p_decl->getLocation() );
    
    if( isdeclinsource ) {
      clang::RecursiveASTVisitor< MyRecursiveASTVisitor >::TraverseDecl( p_decl );
    }
    
    return true;
  }

  
  /**
   * CXX Class/Structs/Union/Enum
   * http://clang.llvm.org/doxygen/classclang_1_1CXXRecordDecl.html
   */
  bool VisitCXXRecordDecl( clang::CXXRecordDecl * p_decl )
  {
    if( ! p_decl->isLambda() ) {    
      // Class, struct, union or enum ?
      switch ( p_decl->getTagKind() ) {
      case clang::TTK_Struct :
	operands[ "struct" ] += 1;
	break;
      case clang::TTK_Class:
	operands[ "class" ] += 1;
	break;
      case clang::TTK_Enum:
	operands[ "enum" ] += 1;
	break;
      case clang::TTK_Union:
	operands[ "union" ] += 1;
	break;
      default:
	break;
      }
    
      // Get the name of the class/struct/enum/union if not anonymous.
      if( ! p_decl->isAnonymousStructOrUnion() ) {
	operands[ p_decl->getNameAsString() ] += 1;
      }
    
      // Process class/struct ancestors if any.
      for( auto & base : p_decl->bases() ) {

	// If virtual ancestor then add "virtual" keyword as operand.
	if( base.isVirtual() ) {
	  operands[ "virtual" ] += 1;
	}

	// Add ancestor access specifier as an operand.
	// TODO: Determine if the specifier "private" is explicitly given since
	// it is implicitly added by the compiler if no specifier is given.
	switch ( base.getAccessSpecifier() ) {
	case clang::AS_public:
	  operands[ "public" ] += 1;
	  break;
	case clang::AS_protected:
	  operands[ "protected" ] += 1;
	  break;
	case clang::AS_private:
	  operands[ "private" ] += 1;
	  break;
	default:
	  break;
	}
      
	std::string str = base.getType().getAsString();
	operands[ str.substr( str.find(' ') + 1 ) ] += 1;
      }
    }
    
    return true;
  }


  /**
   * Method/Constructor/Destructor visitor.
   */
  bool VisitCXXMethodDecl( clang::CXXMethodDecl * p_decl )
  {
    if( p_decl->isConst() ) {
      operands[ "const" ] += 1;
    }
    if( p_decl->isVolatile() ) {
      operands[ "volatile" ] += 1;
    }

    return true;
  }
  

  /**
   * Constructor visitor.
   * Note that, a constructor is considered as a method thus VisitCXXMethodDecl is called
   * on the constructor prior to VisitCXXConstructorDecl.
   */
  bool VisitCXXConstructorDecl( clang::CXXConstructorDecl * p_decl )
  {
    // is explicit ?
    if( p_decl->isExplicit() ) {
      operands[ "explicit" ] += 1;
    }

    // TODO: add ":" as operator ?
    
    // initializers
    for( auto const & init : p_decl->inits() ) {
      operands[ init->getMember()->getNameAsString() ] += 1;
      operators[ "()" ] += 1;
    }
    
    return true;
  }
  

  /**
   * Variable declaration.
   */
  bool VisitVarDecl( clang::VarDecl * p_decl )
  {
    // static C
    if( p_decl->isModulePrivate() ) {
      operands[ "static" ] += 1;
    }

    // static C++
    switch( p_decl->getStorageClass() ) {
    case clang::SC_Static:
      operands[ "static" ] += 1;
      break;
    default:
      break;
    }

    // variable name.
    operands[ p_decl->getNameAsString() ] += 1;
    // type name.
    if( p_decl->getType().getTypePtr()->getContainedAutoType() ) {
      operands[ "auto" ] += 1;
    }
    else {
      operands[ p_decl->getType().getAsString() ] += 1;
    }

    // type var = init ?
    if( p_decl->hasInit() ) {
      operators[ "=" ] += 1;
    }
    
    return true;
  }


  /**
   * Access specifier declaration: public, private, protected.
   */
  bool VisitAccessSpecDecl( clang::AccessSpecDecl * p_decl )
  {
    switch ( p_decl->getAccess() ) {
    case clang::AS_public:
      operands[ "public" ] += 1;
      break;
    case clang::AS_protected:
      operands[ "protected" ] += 1;
      break;
    case clang::AS_private:
      operands[ "private" ] += 1;
      break;
    default:
      break;
    }
    
    return true;
  }

  
  /**
   * Function/Method/Constructor/Destructor declaration.
   */
  bool VisitFunctionDecl( clang::FunctionDecl * p_decl )
  { 
    operands[ p_decl->getNameAsString() ] += 1;

    // if not a constructor or a destructor then get the return type.
    if( ! ( clang::isa< clang::CXXConstructorDecl >( p_decl ) || clang::isa< clang::CXXDestructorDecl >( p_decl ) ) ) {
      operands[ p_decl->getReturnType().getAsString() ] += 1;
    }
    
    // if not a method and not a global function then it is a static function.
    if( ! ( clang::isa< clang::CXXMethodDecl >( p_decl ) || p_decl->isGlobal() ) ) {
      operands[ "static" ] += 1;
    }

    return true;
  }


  /**
   * Field declaration.
   */
  bool VisitFieldDecl( clang::FieldDecl * p_decl)
  {
    operands[ p_decl->getType().getAsString() ] += 1;

    operands[ p_decl->getNameAsString() ] += 1;

    return true;
  }


  /**
   * Return statement declaration.
   */
  bool VisitReturnStmt( clang::ReturnStmt * p_stmt )
  {
    operators[ "return" ] += 1;

    return true;
  }


  /**
   * Literals.
   */
  
  /**
   * TODO: get the original representation instead of a value.
   * "0x10" in the code is translated to "16" with the toString method,
   * thus, the tokens "0x10" and "16" are considered as the same operand.
   */
  bool VisitIntegerLiteral( clang::IntegerLiteral * p_lit )
  {
    operands[ p_lit->getValue().toString( 10, false ) ] += 1;
    
    return true;
  }

  
  bool VisitCharacterLiteral( clang::CharacterLiteral * p_lit )
  {
    operands[ std::to_string( p_lit->getValue() ) ] += 1;
    
    return true;
  }
  

  bool VisitStringLiteral( clang::StringLiteral * p_lit)
  {
    operands[ p_lit->getString().str() ] += 1;
    
    return true;
  }

  
  bool VisitFloatingLiteral( clang::FloatingLiteral * p_lit )
  {
    operands[ std::to_string( p_lit->getValueAsApproximateDouble() ) ] += 1;

    return true;
  }

  
  bool VisitCXXBoolLiteralExpr( clang::CXXBoolLiteralExpr * p_expr )
  {
    if( p_expr->getValue() ) {
      operands[ "true" ] += 1;
    }
    else {
      operands[ "false" ] += 1;
    }
    
    return true;
  }
  

  /**
   * nullptr.
   */
  bool VisitCXXNullPtrLiteralExpr( clang::CXXNullPtrLiteralExpr * p_expr )
  {
    operands[ "nullptr" ] += 1;

    return true;
  }


  /**
   *
   */
  bool VisitDeclRefExpr( clang::DeclRefExpr * p_expr )
  {
    // Function calls are handled by VisitCallExpr.
    if( ! p_expr->getDecl()->getAsFunction() ) {
      operands[ p_expr->getNameInfo().getName().getAsString() ] += 1;
    }
    
    return true;
  }

  
  /**
   * Function calls.
   */
  bool VisitCallExpr( clang::CallExpr * p_expr )
  {
    if( auto callee = p_expr->getDirectCallee() ) {
      operands[ callee->getNameAsString() ] += 1;
      operators[ "()"] += 1;
    }

    return true;
  }


  /**
   * OPERATORS
   */

  
  /**
   * Unary operator.
   */
  bool VisitUnaryOperator( clang::UnaryOperator * p_op )
  {
    operators[ clang::UnaryOperator::getOpcodeStr( p_op->getOpcode() ).str() ] += 1;
    
    return true;
  }
  
  
  /**
   * Binary operator.
   */
  bool VisitBinaryOperator( clang::BinaryOperator * p_op )
  {
    operators[ clang::BinaryOperator::getOpcodeStr( p_op->getOpcode() ).str() ] += 1;
    
    return true;
  }


  /**
   * TEMPLATES
   */

  /**
   * Template parameter.
   */
  bool VisitTemplateTypeParmDecl( clang::TemplateTypeParmDecl * p_decl )
  {
    // template parameter is declared as typename or class.
    if( p_decl->wasDeclaredWithTypename() ) {
      operands[ "typename" ] += 1;
    }
    else {
      operands[ "class" ] += 1;
    }

    // template parameter name.
    operands[ p_decl->getNameAsString() ] += 1;
    
    return true;
  }


  /**
   * Function template.
   */
  bool VisitFunctionTemplateDecl( clang::FunctionTemplateDecl * p_decl )
  {
    operands[ "template" ] += 1;
    operators[ "<>" ] += 1;
    
    return true;
  }

  
  /**
   * Class template.
   */
  bool VisitClassTemplateDecl( clang::ClassTemplateDecl * p_decl )
  {
    operators[ "template" ] += 1;
    operators[ "<>" ] += 1;
    
    return true;
  }

  
  /**
   * USING
   * TODO: detect the typename keyword in using declarations.
   */

  /**
   * using.
   * TODO: getQualifiedNameAsString() does not return the type written
   * by the developer but the one available once the using declaration is evaluated. 
   */
  bool VisitUsingDecl( clang::UsingDecl * p_decl )
  {
    operands[ "using" ] += 1;
    operands[ p_decl->getQualifiedNameAsString() ] += 1;
      
    return true;
  }

  
  bool VisitUsingDirectiveDecl( clang::UsingDirectiveDecl * p_decl )
  {
    operands[ "using" ] += 1;
    operands[ "namespace" ] += 1;
    operands[ p_decl->getNominatedNamespace()->getNameAsString() ] += 1;
    
    return true;
  }

  
  /**
   * Type alias: "using type = underlyingtype;"
   */
  bool VisitTypeAliasDecl( clang::TypeAliasDecl * p_decl )
  {
    operands[ "using" ] += 1;
    operands[ p_decl->getNameAsString() ] += 1;
    operators[ "=" ] += 1;
    operands[ p_decl->getUnderlyingType().getAsString() ] += 1;
    
    return true;
  }

  
  /**
   * Template alias: "template < ... > using type< ... > = underlyingtype< ... >;"
   * TODO: to be continued...
   */
  bool VisitTypeAliasTemplateDecl( clang::TypeAliasTemplateDecl * p_decl )
  {
    operands[ "template" ] += 1;

    return 0;
  }


  /**
   * CONTROL STATEMENTS.
   */

  
  /**
   * If.
   */
  bool VisitIfStmt( clang::IfStmt * p_stmt )
  {
    operands[ "if" ] += 1;
    operators[ "()" ] += 1;

    if( p_stmt->getElse() ) {
      operands[ "else" ] += 1;
    }
    
    return true;
  }

  
  /**
   * For.
   */
  bool VisitForStmt( clang::ForStmt * p_stmt )
  {
    operands[ "for" ] += 1;
    operators[ "()" ] += 1;
    
    return true;
  }

  
  /**
   * While.
   */
  bool VisitWhileStmt( clang::WhileStmt * p_stmt )
  {
    operands[ "while" ] += 1;
    operators[ "()" ] += 1;

    return true;
  }

  /**
   * Do while.
   */
  bool VisitDoStmt( clang::DoStmt * p_stmt )
  {
    operands[ "do" ] += 1;
    operands[ "while" ] += 1;
    operators[ "()" ] += 1;

    return true;
  }


  /**
   * LAMBDAS.
   */
  bool VisitLambdaExpr( clang::LambdaExpr * p_expr )
  {
    operators[ "[]" ] += 1;
    operators[ "()" ] += 1;

    if( p_expr->getCaptureDefault() == clang::LCD_ByRef ) {
      operators[ "&" ] += 1;
    }
    
    for( auto const & capture : p_expr->captures() ) {

      if( capture.capturesVariable() ) {
	switch( capture.getCaptureKind() ) {
	case clang::LCK_ByRef:
	  operators[ "&" ] += 1;
	  break;
	}
	operands[ capture.getCapturedVar()->getNameAsString() ] += 1;
      }

    }
    
    return true;
  }
  
  
};


Map MyRecursiveASTVisitor::operators;

Map MyRecursiveASTVisitor::operands;

std::vector< std::string > MyRecursiveASTVisitor::sourcepathlist;


#endif
