/* 
 * EQUATION PARSER 
 * évaluateur d'équations mathématiques personnalisées
 * equation_parser.h   V0.2
 */

#ifndef EQUATION_PARSER_H
#define EQUATION_PARSER_H

#include <Arduino.h>
#include "config.h"
#include "logging.h"

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
    LOG_V(LOG_TIMER, "Expression originale: %s", expr.c_str());
    
    String original = expr;
    expr.replace("waterTemp", String(waterTemp, 2));
    expr.replace("extTemp", String(extTemp, 2));
    expr.replace("weatherMax", String(weatherMax, 2));
    expr.replace("weatherMin", String(weatherMin, 2));
    expr.replace("sunshine", String(sunshine, 2));
    
    if (original != expr) {
      LOG_D(LOG_TIMER, "Variables remplacees:");
      LOG_V(LOG_TIMER, "  waterTemp = %.2f", waterTemp);
      LOG_V(LOG_TIMER, "  extTemp = %.2f", extTemp);
      LOG_V(LOG_TIMER, "  weatherMax = %.2f", weatherMax);
      LOG_V(LOG_TIMER, "  weatherMin = %.2f", weatherMin);
      LOG_V(LOG_TIMER, "  sunshine = %.2f", sunshine);
      LOG_V(LOG_TIMER, "Expression apres substitution: %s", expr.c_str());
    }
    
    return expr;
  }
  
  // Trouver un nombre à partir d'une position
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
      LOG_E(LOG_TIMER, "Erreur extraction nombre a la position %d", pos);
      error = true;
      return 0;
    }
    
    float result = num.toFloat();
    result = isNegative ? -result : result;
    LOG_V(LOG_TIMER, "Nombre extrait a pos %d: %.2f", pos, result);
    return result;
  }
  
  // évaluer une expression (sans parenthèses)
  float evaluateSimple(String expr, bool& error) {
    expr.trim();
    
    // Supprimer les espaces
    expr.replace(" ", "");
    
    if (expr.length() == 0) {
      LOG_E(LOG_TIMER, "Expression vide");
      error = true;
      return 0;
    }
    
    LOG_V(LOG_TIMER, "Evaluation simple: %s", expr.c_str());
    
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
      if (error) {
        LOG_E(LOG_TIMER, "Erreur lors de l'extraction du nombre %d", numCount);
        return 0;
      }
      
      if (pos < expr.length()) {
        char op = expr.charAt(pos);
        if (op == '+' || op == '-' || op == '*' || op == '/') {
          operators[opCount++] = op;
          LOG_V(LOG_TIMER, "Operateur trouve: %c", op);
          pos++;
        } else {
          LOG_E(LOG_TIMER, "Operateur invalide a la position %d: %c", pos, op);
          error = true;
          return 0;
        }
      }
    }
    
    LOG_V(LOG_TIMER, "Parse termine: %d nombres, %d operateurs", numCount, opCount);
    
    // Appliquer * et / d'abord (priorité)
    for (int i = 0; i < opCount; i++) {
      if (operators[i] == '*' || operators[i] == '/') {
        float result;
        if (operators[i] == '*') {
          result = numbers[i] * numbers[i + 1];
          LOG_V(LOG_TIMER, "Multiplication: %.2f * %.2f = %.2f", 
                numbers[i], numbers[i + 1], result);
        } else {
          if (numbers[i + 1] == 0) {
            LOG_E(LOG_TIMER, "Division par zero detectee");
            error = true;
            return 0;
          }
          result = numbers[i] / numbers[i + 1];
          LOG_V(LOG_TIMER, "Division: %.2f / %.2f = %.2f", 
                numbers[i], numbers[i + 1], result);
        }
        numbers[i] = result;
        
        // Décaler les tableaux
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
    LOG_V(LOG_TIMER, "Valeur initiale: %.2f", result);
    
    for (int i = 0; i < opCount; i++) {
      if (operators[i] == '+') {
        float before = result;
        result += numbers[i + 1];
        LOG_V(LOG_TIMER, "Addition: %.2f + %.2f = %.2f", before, numbers[i + 1], result);
      } else if (operators[i] == '-') {
        float before = result;
        result -= numbers[i + 1];
        LOG_V(LOG_TIMER, "Soustraction: %.2f - %.2f = %.2f", before, numbers[i + 1], result);
      }
    }
    
    LOG_V(LOG_TIMER, "Resultat evaluation simple: %.2f", result);
    return result;
  }
  
  float evaluateWithParentheses(String expr, bool& error) {
    expr.trim();
    
    LOG_V(LOG_TIMER, "Evaluation avec parentheses: %s", expr.c_str());
    
    int parenthesesCount = 0;
    
    // Traiter les parenthèses de l'intérieur vers l'extérieur
    while (expr.indexOf('(') >= 0) {
      parenthesesCount++;
      int lastOpen = expr.lastIndexOf('(');
      int firstClose = expr.indexOf(')', lastOpen);
      
      if (firstClose < 0) {
        LOG_E(LOG_TIMER, "Parenthese ouvrante sans parenthese fermante");
        error = true;
        return 0;
      }
      
      String subExpr = expr.substring(lastOpen + 1, firstClose);
      LOG_V(LOG_TIMER, "Evaluation sous-expression #%d: (%s)", parenthesesCount, subExpr.c_str());
      
      float subResult = evaluateSimple(subExpr, error);
      
      if (error) {
        LOG_E(LOG_TIMER, "Erreur dans la sous-expression: %s", subExpr.c_str());
        return 0;
      }
      
      LOG_V(LOG_TIMER, "Resultat sous-expression: %.2f", subResult);
      
      expr = expr.substring(0, lastOpen) + String(subResult, 6) + expr.substring(firstClose + 1);
      LOG_V(LOG_TIMER, "Expression apres substitution: %s", expr.c_str());
    }
    
    // Vérifier les parenthèses fermantes sans ouvrantes
    if (expr.indexOf(')') >= 0) {
      LOG_E(LOG_TIMER, "Parenthese fermante sans parenthese ouvrante");
      error = true;
      return 0;
    }
    
    LOG_D(LOG_TIMER, "Toutes les parentheses traitees (%d niveaux), evaluation finale", parenthesesCount);
    return evaluateSimple(expr, error);
  }

public:
  void setVariables(float wTemp, float eTemp, float wMax, float wMin, float sun) {
    waterTemp = wTemp;
    extTemp = eTemp;
    weatherMax = wMax;
    weatherMin = wMin;
    sunshine = sun;
    
    LOG_V(LOG_TIMER, "Variables d'equation initialisees:");
    LOG_V(LOG_TIMER, "  waterTemp=%.2f, extTemp=%.2f", wTemp, eTemp);
    LOG_V(LOG_TIMER, "  weatherMax=%.2f, weatherMin=%.2f, sunshine=%.2f", wMax, wMin, sun);
  }
  
  float calculate(String expr, bool& error) {
    error = false;
    
    LOG_SEPARATOR();
    LOG_I(LOG_TIMER, "Calcul d'equation demarre");
    LOG_I(LOG_TIMER, "Expression: %s", expr.c_str());
    
    // Remplacer les variables
    expr = replaceVariables(expr);
    
    // Calculer
    float result = evaluateWithParentheses(expr, error);
    
    if (error) {
      LOG_E(LOG_TIMER, "ERREUR: Echec du calcul de l'equation");
      LOG_SEPARATOR();
      return 0;
    }
    
    LOG_I(LOG_TIMER, "Calcul termine avec succes");
    LOG_I(LOG_TIMER, "Resultat final: %.2f", result);
    LOG_SEPARATOR();
    
    return result;
  }
  
  // Validation de la syntaxe
  static bool validate(String expr) {
    LOG_D(LOG_TIMER, "Validation de syntaxe: %s", expr.c_str());
    
    expr.trim();
    expr.replace(" ", "");
    
    if (expr.length() == 0) {
      LOG_E(LOG_TIMER, "Validation echouee: expression vide");
      return false;
    }
    
    // Vérifier les parenthèses
    int parentheses = 0;
    for (int i = 0; i < expr.length(); i++) {
      if (expr.charAt(i) == '(') parentheses++;
      if (expr.charAt(i) == ')') parentheses--;
      if (parentheses < 0) {
        LOG_E(LOG_TIMER, "Validation echouee: parenthese fermante sans ouvrante a la position %d", i);
        return false;
      }
    }
    
    if (parentheses != 0) {
      LOG_E(LOG_TIMER, "Validation echouee: %d parenthese(s) non fermee(s)", parentheses);
      return false;
    }
    
    // Vérifier les caractères valides
    for (int i = 0; i < expr.length(); i++) {
      char c = expr.charAt(i);
      bool valid = isDigit(c) || c == '.' || c == '+' || c == '-' || 
                   c == '*' || c == '/' || c == '(' || c == ')' ||
                   isAlpha(c); // Pour les noms de variables
      if (!valid) {
        LOG_E(LOG_TIMER, "Validation echouee: caractere invalide '%c' a la position %d", c, i);
        return false;
      }
    }
    
    LOG_I(LOG_TIMER, "Validation reussie");
    return true;
  }
};

#endif