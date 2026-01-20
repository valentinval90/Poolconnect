/* 
 * EQUATION PARSER 
 * évaluateur d'équations mathématiques personnalisées
 */

#ifndef EQUATION_PARSER_H
#define EQUATION_PARSER_H

#include <Arduino.h>

class EquationParser {
private:
  String expression;
  float waterTemp;
  float extTemp;
  float weatherMax;
  float weatherMin;
  float sunshine;
  
  // Remplacer les variables par leurs valeurs
  String replaceVariables(String expr) {
    expr.replace("waterTemp", String(waterTemp, 2));
    expr.replace("extTemp", String(extTemp, 2));
    expr.replace("weatherMax", String(weatherMax, 2));
    expr.replace("weatherMin", String(weatherMin, 2));
    expr.replace("sunshine", String(sunshine, 2));
    return expr;
  }
  
  // Trouver un nombre &  partir d'une position
  float extractNumber(String expr, int& pos, bool& error) {
    String num = "";
    bool hasDecimal = false;
    bool isNegative = false;
    
    // Gérer le signe négatif
    if (pos < expr.length() && expr.charAt(pos) == '-') {
      isNegative = true;
      pos++;
    }
    
    while (pos < expr.length()) {
      char c = expr.charAt(pos);
      if (isDigit(c)) {
        num += c;
      } else if (c == '.' && !hasDecimal) {
        num += c;
        hasDecimal = true;
      } else {
        break;
      }
      pos++;
    }
    
    if (num.length() == 0 || num == ".") {
      error = true;
      return 0;
    }
    
    float result = num.toFloat();
    return isNegative ? -result : result;
  }
  
  // évaluer une expression (sans parenthéses)
  float evaluateSimple(String expr, bool& error) {
    expr.trim();
    
    // Supprimer les espaces
    expr.replace(" ", "");
    
    if (expr.length() == 0) {
      error = true;
      return 0;
    }
    
    // Tableaux pour stocker les nombres et opérateurs
    float numbers[50];
    char operators[50];
    int numCount = 0;
    int opCount = 0;
    
    int pos = 0;
    
    // Parser l'expression
    while (pos < expr.length()) {
      // Extraire un nombre
      numbers[numCount++] = extractNumber(expr, pos, error);
      if (error) return 0;
      
      if (pos < expr.length()) {
        char op = expr.charAt(pos);
        if (op == '+' || op == '-' || op == '*' || op == '/') {
          operators[opCount++] = op;
          pos++;
        } else {
          error = true;
          return 0;
        }
      }
    }
    
    for (int i = 0; i < opCount; i++) {
      if (operators[i] == '*' || operators[i] == '/') {
        if (operators[i] == '*') {
          numbers[i] = numbers[i] * numbers[i + 1];
        } else {
          if (numbers[i + 1] == 0) {
            error = true;
            return 0;
          }
          numbers[i] = numbers[i] / numbers[i + 1];
        }
        
        for (int j = i + 1; j < numCount - 1; j++) {
          numbers[j] = numbers[j + 1];
        }
        for (int j = i; j < opCount - 1; j++) {
          operators[j] = operators[j + 1];
        }
        numCount--;
        opCount--;
        i--;
      }
    }
    
    // Appliquer + et -
    float result = numbers[0];
    for (int i = 0; i < opCount; i++) {
      if (operators[i] == '+') {
        result += numbers[i + 1];
      } else if (operators[i] == '-') {
        result -= numbers[i + 1];
      }
    }
    
    return result;
  }
  
  float evaluateWithParentheses(String expr, bool& error) {
    expr.trim();
    
    while (expr.indexOf('(') >= 0) {
      int lastOpen = expr.lastIndexOf('(');
      int firstClose = expr.indexOf(')', lastOpen);
      
      if (firstClose < 0) {
        error = true;
        return 0;
      }
      
      String subExpr = expr.substring(lastOpen + 1, firstClose);
      float subResult = evaluateSimple(subExpr, error);
      
      if (error) return 0;
      
      expr = expr.substring(0, lastOpen) + String(subResult, 6) + expr.substring(firstClose + 1);
    }
    
    if (expr.indexOf(')') >= 0) {
      error = true;
      return 0;
    }
    
    return evaluateSimple(expr, error);
  }

public:
  void setVariables(float wTemp, float eTemp, float wMax, float wMin, float sun) {
    waterTemp = wTemp;
    extTemp = eTemp;
    weatherMax = wMax;
    weatherMin = wMin;
    sunshine = sun;
  }
  
  float calculate(String expr, bool& error) {
    error = false;
    
    // Remplacer les variables
    expr = replaceVariables(expr);
    
    return evaluateWithParentheses(expr, error);
  }
  
  // Validation de la syntaxe
  static bool validate(String expr) {
    expr.trim();
    expr.replace(" ", "");
    
    if (expr.length() == 0) return false;
    
    int parentheses = 0;
    for (int i = 0; i < expr.length(); i++) {
      if (expr.charAt(i) == '(') parentheses++;
      if (expr.charAt(i) == ')') parentheses--;
      if (parentheses < 0) return false;
    }
    
    if (parentheses != 0) return false;
    
    for (int i = 0; i < expr.length(); i++) {
      char c = expr.charAt(i);
      bool valid = isDigit(c) || c == '.' || c == '+' || c == '-' || 
                   c == '*' || c == '/' || c == '(' || c == ')' ||
                   isAlpha(c); // Pour les noms de variables
      if (!valid) return false;
    }
    
    return true;
  }
};

#endif