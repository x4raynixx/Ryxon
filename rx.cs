using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Text;
using System.Text.RegularExpressions;
struct BlockInfo
{
    public string RawCode;
    public int EndLineIndex;
    public bool Found;
}


public class RXInterpreter
{
    static Dictionary<string, int> variables = new Dictionary<string, int>();
    static Random random = new Random();
    static string PreprocessLine(string line)
    {
        StringBuilder processedLine = new StringBuilder(line);
        int currentPos = 0;
        while (currentPos < processedLine.Length)
        {
            int commentStart = -1;
            for (int i = currentPos; i < processedLine.Length - 1; i++)
            {
                if (processedLine[i] == '|' && processedLine[i + 1] == '|')
                {
                    commentStart = i;
                    break;
                }
            }

            if (commentStart == -1)
            {
                break;
            }

            int commentEnd = -1;
            for (int i = commentStart + 2; i < processedLine.Length - 1; i++)
            {
                 if (processedLine[i] == '|' && processedLine[i + 1] == '|')
                 {
                     commentEnd = i;
                     break;
                 }
            }

            if (commentEnd != -1)
            {
                processedLine.Remove(commentStart, commentEnd - commentStart + 2);
                currentPos = commentStart;
            }
            else
            {
                processedLine.Remove(commentStart, processedLine.Length - commentStart);
                break;
            }
        }
        return processedLine.ToString();
    }


    static string FormatString(string str)
    {
        string result = str;
        Regex rgx = new Regex("%\\((.*?)\\)");
        MatchCollection matches = rgx.Matches(result);

        for (int i = matches.Count - 1; i >= 0; i--)
        {
            Match match = matches[i];
            string varName = match.Groups[1].Value;
            string replacement;
            if (variables.TryGetValue(varName, out int varValue))
            {
                replacement = varValue.ToString();
            }
            else
            {
                replacement = "undefined";
            }
            result = result.Remove(match.Index, match.Length).Insert(match.Index, replacement);
        }
        return result;
    }


    static int randomInRange(int min, int max)
    {
        if (min > max) {
             int temp = min;
             min = max;
             max = temp;
        }
        return random.Next(min, max + 1);
    }

    static void PrintError(string message, int lineNumber, string filename)
    {
        Console.ForegroundColor = ConsoleColor.Red;
        Console.Error.Write($"Error");
        if (lineNumber > 0) {
             Console.Error.Write($" at line {lineNumber}");
        }
        if (!string.IsNullOrEmpty(filename) && filename != "unknown") {
            Console.Error.Write($" in file {filename}");
        }
        Console.Error.WriteLine($": {message}");
        Console.ResetColor();
    }

    static int EvaluateExpression(string expression, int lineNumber, string filename)
    {
        expression = expression.Trim();

        try
        {
            int parenLevel = 0;
            int splitPos = -1;
             for (int i = expression.Length - 1; i >= 0; i--)
            {
                char c = expression[i];
                if (c == ')') parenLevel++;
                else if (c == '(') parenLevel--;
                else if (parenLevel == 0 && (c == '+' || c == '-'))
                {
                    splitPos = i;
                    break;
                }
            }


            if (splitPos != -1)
            {
                char op = expression[splitPos];
                string left = expression.Substring(0, splitPos);
                string right = expression.Substring(splitPos + 1);
                int leftVal = EvaluateExpression(left, lineNumber, filename);
                int rightVal = EvaluateExpression(right, lineNumber, filename);
                return op == '+' ? leftVal + rightVal : leftVal - rightVal;
            }

            parenLevel = 0;
            splitPos = -1;
            for (int i = expression.Length - 1; i >= 0; i--)
            {
                char c = expression[i];
                if (c == ')') parenLevel++;
                else if (c == '(') parenLevel--;
                else if (parenLevel == 0 && (c == '*' || c == '/'))
                {
                    splitPos = i;
                    break;
                }
            }

            if (splitPos != -1)
            {
                char op = expression[splitPos];
                string left = expression.Substring(0, splitPos);
                string right = expression.Substring(splitPos + 1);
                int leftVal = EvaluateExpression(left, lineNumber, filename);
                int rightVal = EvaluateExpression(right, lineNumber, filename);
                if (op == '*') return leftVal * rightVal;
                if (op == '/')
                {
                    if (rightVal == 0)
                    {
                        PrintError("Division by zero", lineNumber, filename);
                        throw new DivideByZeroException();
                    }
                    return leftVal / rightVal;
                }
            }

            if (expression.StartsWith("(") && expression.EndsWith(")"))
            {
                parenLevel = 0;
                bool matching = true;
                for(int i = 0; i < expression.Length; i++) {
                    if (expression[i] == '(') parenLevel++;
                    else if (expression[i] == ')') parenLevel--;
                    if (parenLevel == 0 && i < expression.Length - 1) {
                        matching = false;
                        break;
                    }
                    if (parenLevel < 0) {
                         matching = false;
                         break;
                    }
                }
                if (matching && parenLevel == 0) {
                    return EvaluateExpression(expression.Substring(1, expression.Length - 2), lineNumber, filename);
                }
            }
            if (int.TryParse(expression, out int number))
            {
                return number;
            }
            if (variables.TryGetValue(expression, out int varValue))
            {
                return varValue;
            }

            PrintError($"Invalid expression or undefined variable: {expression}", lineNumber, filename);
            throw new ArgumentException($"Invalid expression or undefined variable: {expression}");
        }
        catch (Exception ex)
        {
             if (!(ex is DivideByZeroException || ex is ArgumentException)) {
                 PrintError($"Error evaluating expression '{expression}': {ex.Message}", lineNumber, filename);
             }
             throw;
        }
    }


    static bool EvaluateCondition(string condition, int lineNumber, string filename)
    {
        condition = condition.Trim();
        string[] operators = { "==", "!=", "<=", ">=", "<", ">" };

        foreach (string op in operators)
        {
            int opPos = condition.IndexOf(op);
            if (opPos != -1)
            {
                string leftStr = condition.Substring(0, opPos).Trim();
                string rightStr = condition.Substring(opPos + op.Length).Trim();

                try
                {
                    int leftVal = EvaluateExpression(leftStr, lineNumber, filename);
                    int rightVal = EvaluateExpression(rightStr, lineNumber, filename);

                    switch (op)
                    {
                        case "==": return leftVal == rightVal;
                        case "!=": return leftVal != rightVal;
                        case "<": return leftVal < rightVal;
                        case ">": return leftVal > rightVal;
                        case "<=": return leftVal <= rightVal;
                        case ">=": return leftVal >= rightVal;
                    }
                }
                catch (Exception ex)
                {
                    PrintError($"Error evaluating condition '{condition}': {ex.Message}", lineNumber, filename);
                    return false;
                }
            }
        }

        PrintError($"Invalid condition syntax: {condition}", lineNumber, filename);
        return false;
    }

    static BlockInfo ExtractBlockOriginalLogic(List<string> codeLines, int startLineIdx, int startBraceCharIdx, string filename)
    {
        StringBuilder blockCodeBuilder = new StringBuilder();
        int braceLevel = 0;
        int blockEndLineIndex = -1;
        bool blockFound = false;
        int currentParseLineIndex = startLineIdx;
        int searchStartIndexOnFirstLine = startBraceCharIdx + 1;

        while (currentParseLineIndex < codeLines.Count)
        {
            string currentParseLine = codeLines[currentParseLineIndex];
            int currentParseCharIndex = (currentParseLineIndex == startLineIdx) ? searchStartIndexOnFirstLine : 0;

            for (int j = currentParseCharIndex; j < currentParseLine.Length; j++)
            {
                char c = currentParseLine[j];
                if (c == '{')
                {
                    if (braceLevel >= 0) blockCodeBuilder.Append(c);
                    braceLevel++;
                }
                else if (c == '}')
                {
                    if (braceLevel == 0)
                    {
                        blockEndLineIndex = currentParseLineIndex;
                        blockFound = true;
                        goto EndBlockSearchHelper;
                    }
                    braceLevel--;
                    if (braceLevel >= 0) blockCodeBuilder.Append(c);
                    else
                    {
                        PrintError($"Mismatched closing brace '}}' found while parsing block starting near line {startLineIdx + 1}", currentParseLineIndex + 1, filename);
                        return new BlockInfo { Found = false };
                    }
                }
                else if (braceLevel >= 0)
                {
                    blockCodeBuilder.Append(c);
                }
            }

            currentParseLineIndex++;
            if (currentParseLineIndex < codeLines.Count && !blockFound && braceLevel >= 0)
            {
                blockCodeBuilder.AppendLine();
            }
        }

    EndBlockSearchHelper:

        if (!blockFound)
        {
            PrintError($"Mismatched opening brace '{{' for block starting on line {startLineIdx + 1}", startLineIdx + 1, filename);
            return new BlockInfo { Found = false };
        }

        return new BlockInfo { RawCode = blockCodeBuilder.ToString(), EndLineIndex = blockEndLineIndex, Found = true };
    }


    static void ExecuteRX(string code, string filename)
    {
        List<string> lines = new List<string>();
        using (StringReader reader = new StringReader(code))
        {
            string tempLine;
            while ((tempLine = reader.ReadLine()) != null)
            {
                lines.Add(tempLine);
            }
        }

        int i = 0;
        while (i < lines.Count)
        {
            int currentLineNumber = i + 1;
            string originalLine = lines[i];
            string line = PreprocessLine(originalLine);
            string trimmedLine = line.TrimStart();

            if (trimmedLine.StartsWith("if"))
            {
                int openParen = originalLine.IndexOf('(');
                int closeParen = -1;
                if (openParen != -1) closeParen = originalLine.IndexOf(')', openParen);
                int openBrace = -1;
                if (closeParen != -1) openBrace = originalLine.IndexOf('{', closeParen);

                if (openParen == -1 || closeParen == -1 || openBrace == -1 || !(closeParen > openParen && openBrace > closeParen))
                {
                    PrintError("Invalid if statement syntax 'if (condition) {'", currentLineNumber, filename);
                    i++;
                    continue;
                }

                string condition = originalLine.Substring(openParen + 1, closeParen - openParen - 1);
                BlockInfo ifBlockInfo = ExtractBlockOriginalLogic(lines, i, openBrace, filename);
                if (!ifBlockInfo.Found)
                {
                    i = lines.Count;
                    continue;
                }

                bool conditionResult = false;
                try {
                    conditionResult = EvaluateCondition(condition, currentLineNumber, filename);
                } catch (Exception) {
                }

                bool blockExecuted = false;
                int currentChainEndLineIndex = ifBlockInfo.EndLineIndex;

                if (conditionResult)
                {
                    try
                    {
                        ExecuteRX(ifBlockInfo.RawCode, filename);
                    }
                    catch (Exception ex)
                    {
                        PrintError($"Error executing if block from line {i + 1}: {ex.Message}", currentLineNumber, filename);
                    }
                    blockExecuted = true;
                }

                while (true)
                {
                    int nextLineIndex = currentChainEndLineIndex + 1;
                    if (nextLineIndex >= lines.Count) break;

                    string nextLineOriginal = lines[nextLineIndex];
                    string nextLinePreprocessed = PreprocessLine(nextLineOriginal);
                    string nextLineTrimmed = nextLinePreprocessed.Trim();

                    if (nextLineTrimmed.StartsWith("else if"))
                    {
                        int nextOpenParen = nextLineOriginal.IndexOf('(');
                        int nextCloseParen = -1;
                        if (nextOpenParen != -1) nextCloseParen = nextLineOriginal.IndexOf(')', nextOpenParen);
                        int nextOpenBrace = -1;
                        if (nextCloseParen != -1) nextOpenBrace = nextLineOriginal.IndexOf('{', nextCloseParen);

                        if (nextOpenParen == -1 || nextCloseParen == -1 || nextOpenBrace == -1 || !(nextCloseParen > nextOpenParen && nextOpenBrace > nextCloseParen))
                        {
                            PrintError("Invalid else if statement syntax 'else if (condition) {'", nextLineIndex + 1, filename);
                            break;
                        }

                        BlockInfo elseIfBlockInfo = ExtractBlockOriginalLogic(lines, nextLineIndex, nextOpenBrace, filename);
                        if (!elseIfBlockInfo.Found)
                        {
                            i = lines.Count;
                            goto EndMainLoop;
                        }

                        if (!blockExecuted)
                        {
                            string elseIfCondition = nextLineOriginal.Substring(nextOpenParen + 1, nextCloseParen - nextOpenParen - 1);
                            bool elseIfConditionResult = false;
                            try {
                                elseIfConditionResult = EvaluateCondition(elseIfCondition, nextLineIndex + 1, filename);
                            } catch (Exception) {
                            }

                            if (elseIfConditionResult)
                            {
                                try
                                {
                                    ExecuteRX(elseIfBlockInfo.RawCode, filename);
                                }
                                catch (Exception ex)
                                {
                                    PrintError($"Error executing else if block from line {nextLineIndex + 1}: {ex.Message}", nextLineIndex + 1, filename);
                                }
                                blockExecuted = true;
                            }
                        }
                        currentChainEndLineIndex = elseIfBlockInfo.EndLineIndex;
                    }
                    else if (nextLineTrimmed.StartsWith("else"))
                    {
                        int nextOpenBrace = nextLineOriginal.IndexOf('{');
                        string elsePart = nextLinePreprocessed.Substring(0, nextOpenBrace < 0 ? nextLinePreprocessed.Length : nextOpenBrace).Trim();
                        if (nextOpenBrace == -1 || elsePart != "else")
                        {
                             PrintError("Invalid else statement syntax 'else {'", nextLineIndex + 1, filename);
                             break;
                        }
                        
                        BlockInfo elseBlockInfo = ExtractBlockOriginalLogic(lines, nextLineIndex, nextOpenBrace, filename);
                        if (!elseBlockInfo.Found)
                        {
                            i = lines.Count;
                            goto EndMainLoop;
                        }
                        
                        if (!blockExecuted)
                        {
                            try
                            {
                                ExecuteRX(elseBlockInfo.RawCode, filename);
                            }
                            catch (Exception ex)
                            {
                                PrintError($"Error executing else block from line {nextLineIndex + 1}: {ex.Message}", nextLineIndex + 1, filename);
                            }
                            blockExecuted = true;
                        }
                        currentChainEndLineIndex = elseBlockInfo.EndLineIndex;
                        break;
                    }
                    else
                    {
                        break;
                    }
                }
                i = currentChainEndLineIndex + 1;
                continue;

            }
            else if (trimmedLine.StartsWith("log("))
            {
                 int argStart = line.IndexOf("log(") + 4;
                 int argEnd = line.LastIndexOf(')');
                 if (argStart >= 4 && argEnd > argStart) { 
                     string logArg = line.Substring(argStart, argEnd - argStart).Trim();
                     if ((logArg.StartsWith("'") && logArg.EndsWith("'")) || (logArg.StartsWith("\"") && logArg.EndsWith("\"")))
                     {
                         string content = logArg.Substring(1, logArg.Length - 2);
                         Console.WriteLine(FormatString(content));
                     }
                     else
                     {
                         try
                         {
                             int result = EvaluateExpression(logArg, currentLineNumber, filename);
                             Console.WriteLine(result);
                         }
                         catch (Exception)
                         {
                         }
                     }
                 } else {
                     PrintError("Invalid syntax in log statement! Expected log('message' or expression)", currentLineNumber, filename);
                 }
            }
            else if (trimmedLine.StartsWith("batch("))
            {
                 int argStart = line.IndexOf("batch(") + 6;
                 int argEnd = line.LastIndexOf(')');
                 if (argStart >= 6 && argEnd > argStart) {
                     string batchArg = line.Substring(argStart, argEnd - argStart).Trim();
                     if ((batchArg.StartsWith("'") && batchArg.EndsWith("'")) || (batchArg.StartsWith("\"") && batchArg.EndsWith("\"")))
                     {
                         string command = batchArg.Substring(1, batchArg.Length - 2);
                         try
                         {
                             ProcessStartInfo psi = new ProcessStartInfo();
                             bool isWindows = System.Runtime.InteropServices.RuntimeInformation.IsOSPlatform(System.Runtime.InteropServices.OSPlatform.Windows);
                             psi.FileName = isWindows ? "cmd.exe" : "/bin/sh";
                             psi.Arguments = isWindows ? $"/c \"{command}\"" : $"-c \"{command}\"";
                             psi.UseShellExecute = false;
                             psi.CreateNoWindow = true;
                             psi.RedirectStandardOutput = true;
                             psi.RedirectStandardError = true;

                             using (Process process = Process.Start(psi))
                             {
                                 process.WaitForExit();
                             }
                         }
                         catch (Exception ex)
                         {
                             PrintError($"Failed to execute batch command: {ex.Message}", currentLineNumber, filename);
                         }
                     }
                     else
                     {
                         PrintError("Batch command must be enclosed in single or double quotes", currentLineNumber, filename);
                     }
                 } else {
                     PrintError("Invalid syntax in batch statement! Expected batch('command')", currentLineNumber, filename);
                 }
            }
            else if (line.Contains("="))
            {
                 int equalsPos = line.IndexOf('=');
                 string varName = line.Substring(0, equalsPos).Trim();
                 string expression = line.Substring(equalsPos + 1).Trim();

                 if (string.IsNullOrWhiteSpace(varName) || !Regex.IsMatch(varName, @"^[a-zA-Z_][a-zA-Z0-9_]*$")) {
                     PrintError($"Invalid variable name: '{varName}'", currentLineNumber, filename);
                     i++;
                     continue;
                 }

                 if (expression.StartsWith("random(") && expression.EndsWith(")"))
                 {
                     string range = expression.Substring(7, expression.Length - 8).Trim();
                     int commaPos = range.IndexOf(',');
                     if (commaPos != -1)
                     {
                         string minStr = range.Substring(0, commaPos).Trim();
                         string maxStr = range.Substring(commaPos + 1).Trim();
                         try {
                             int min = EvaluateExpression(minStr, currentLineNumber, filename);
                             int max = EvaluateExpression(maxStr, currentLineNumber, filename);
                             variables[varName] = randomInRange(min, max);
                         } catch (Exception) {
                              PrintError("Invalid values or variables in random range", currentLineNumber, filename);
                         }

                     }
                     else
                     {
                         PrintError("Invalid range format for random(min, max)", currentLineNumber, filename);
                     }
                 }
                 else
                 {
                     try
                     {
                         int value = EvaluateExpression(expression, currentLineNumber, filename);
                         variables[varName] = value;
                     }
                     catch (Exception)
                     {
                     }
                 }
            }
            else if (!string.IsNullOrWhiteSpace(line))
            {
                 PrintError($"Unknown command or syntax: {line.Trim()}", currentLineNumber, filename);
            }

            i++;
        }

    EndMainLoop:;

    }

    public static void Main(string[] args)
    {
        if (args.Length < 1)
        {
            Console.WriteLine("Usage: rx <filename.rx>");
            return;
        }

        string filename = args[0];
        string code;

        try
        {
            code = File.ReadAllText(filename);
        }
        catch (FileNotFoundException)
        {
            Console.Error.WriteLine($"Error: Cannot open file {filename}");
            return;
        }
        catch (IOException ex)
        {
             Console.Error.WriteLine($"Error reading file {filename}: {ex.Message}");
             return;
        }
        catch (Exception ex)
        {
             Console.Error.WriteLine($"An unexpected error occurred while reading the file: {ex.Message}");
             return;
        }
        try
        {
            ExecuteRX(code, filename);
        }
        catch (Exception ex)
        {
             PrintError($"An unexpected error occurred during execution: {ex.Message}", -1, filename);
        }

    }
}
