<?php

#Author: Vojtech Mimochodek
#Login: xmimoc01
#Date: 11.3.2020
#VUT FIT

/*
Function for generate instructions
*/
function generate_instruction($dom, $program, $words)
{
    global $id;
    global $instruction;
    $id++;
    $instruction = $dom->createElement("instruction");
    $program->appendChild($instruction);
    $order = $dom->createAttribute("order");
    $order->value = $id;
    $instruction->appendChild($order);

    $opcode = $dom->createAttribute("opcode");
    $opcode->value = $words;
    $instruction->appendChild($opcode);
}
/*
Function for generate arguments of instruction
*/
function generate_argument($dom, $type, $num_arg, $atribute)
{
    global $instruction;
    
    $argument = $dom->createElement($num_arg);
    $instruction->appendChild($argument);

    $type_atr = $dom->createAttribute("type");
    $type_atr->value = $type;
    $argument->appendChild($type_atr);
    $argument_text = $dom->createTextNode($atribute);
    $argument->appendChild($argument_text);

}
/*
Function that transform <symb> to correct form.
After that this form is returned and goes to fuction "generate_arguments"
*/
function generate_symb($words)
{
    if (preg_match('/^(L|T|G)F@[[:alnum:]\_\-\$\&\%\*]+/', $words) !== 1) {
        $txt = explode ("@",$words);
        $symb_type = $txt[0];
        unset($txt[0]);
        $txt = implode("@", $txt);
        $return_value = array($symb_type, $txt);
        return $return_value;
    }
    else {
        $return_value = array("var", $words);
        return $return_value;
    }
}
/*
Fuction to parsing input arguments.
Supported arguments: --help / --stats=file / --loc / --comments / --labels / --jumps
*/
function check_arguments($argc, $argv)
{
    global $statistics;
    global $stat_file;
    $print_statistics = array();

    //Help, single --stats or error
    if($argc == 2)
    {
        $stats = explode("=", $argv[1]);
        if($argv[1] === "--help")
        {
            echo "Napoveda:\n";
            echo "Pro zobrazeni napovedy slouzi prepinac --help\n";
            echo "Pro vypsani statistik do souboru behem chodu pouzijte nasledujici prepinace:\n";
            echo "--stats=file = aktivace zapisovani statistik, kde file je nazev souboru pro vypsani statistik\n";
            echo "--loc = vypise do statistik pocet radku s instrukcemi\n";
            echo "--jumps = vypise do statistik pocet skoku\n";
            echo "--labels = vypise do statistik pocet navesti\n";
            echo "--comments = vypise do statistik pocet komentaru\n";
            echo "\n";
            echo "Script obecne slouzi k prekladu IPPcode20 do XML podoby a kontrole lexikalnich a syntaktickych chyb\n";
            echo "Spoustejte script s IPPcode20 na STDIN\n";
            exit(0);
        }
        elseif(sizeof($stats) < 2 || sizeof($stats) > 2)
        {
            fwrite(STDERR, "Wrong arguments!\n");
            exit(10);
        }
        else
        {
            if($stats[1] === "")
            {
                fwrite(STDERR, "Wrong arguments!\n");
                exit(10);
            }
            else
            {
                $file = fopen($stats[1], "w");
                fclose($file);
            }
        }
        return 0;
    }
    //Statistics
    elseif($argc > 2)
    {
        for($i = 1; $i < $argc; $i++)
        {
            $stats = explode("=", $argv[$i]);
            if(count($stats) > 2)
            {
                fwrite(STDERR, "Wrong arguments!\n");
                exit(10);
            }
            elseif(count($stats) < 2)
            {
                if($stats[0] === "--loc")
                {
                    array_push($print_statistics, "loc");
                }
                elseif($stats[0] === "--comments")
                {
                    array_push($print_statistics, "comments");
                }
                elseif($stats[0] === "--labels")
                {
                    array_push($print_statistics, "labels");
                }
                elseif($stats[0] === "--jumps")
                {
                    array_push($print_statistics, "jumps");
                }
                else
                {
                    fwrite(STDERR, "Wrong arguments!\n");
                    exit(10);
                }
            }
            else
            {
                if($stats[0] == "--stats")
                {
                    if($statistics == True)
                    {
                        if($stats[1] === $stat_file)
                        {}
                        else
                        {
                            fwrite(STDERR, "Wrong arguments!\n");
                            exit(10);
                        }

                    }
                    else
                    {
                        $statistics = True;
                        if($stats[1] === "")
                        {
                            fwrite(STDERR, "Wrong arguments!\n");
                            exit(10);
                        }
                        else
                        {
                            $stat_file = $stats[1];
                        }
                    }
                }
                else
                {
                    fwrite(STDERR, "Wrong arguments!\n");
                    exit(10);
                }
            }
        }
        // Control if statistics are enabled and if --stats argument exists
        if($statistics == false)
        {
            fwrite(STDERR, "Missing argument --stats=file\n");
            exit(10);
        }
        else
        {
            return $print_statistics;
        }
    }
}
/*
Functions to check syntax of var, label, symb and type
*/
function check_var($var)
{
    $regex = "/^TF@[a-zA-Z_\-$&%*!?]{1}[a-zA-Z0-9_\-$&%*!?]*$|^LF@[a-zA-Z_\-$&%*!?]{1}[a-zA-Z0-9_\-$&%*!?]*$|^GF@[a-zA-Z_\-$&%*!?]{1}[a-zA-Z0-9_\-$&%*!?]*$/";

    $result = preg_match($regex, $var);

    return $result;
}

function check_label($label)
{
    $regex = "/^[a-zA-Z_\-$&%*!?]{1}[a-zA-Z0-9_\-$&%*!?]*$/";

    $result = preg_match($regex, $label);

    return $result;
}

function check_symb($symb)
{
    $regex = "/^(nil)(@)(nil)$|^(bool)(@)(true|false)$|^(int)(@)(\S*)$|^(string)(@)([\S]*)$|^(GF|LF|TF)(@)([a-zA-Z_\-$&%*!?]+[a-zA-Z0-9_\-$&%*!?]*)$/";

    $result = preg_match($regex, $symb);

    return $result;
}

function check_type($type)
{
    $regex = "/^(int|string|bool)/";

    $result = preg_match($regex, $type);

    return $result;
}
/*
----------
*/

/*
Function that controls syntactical and lexical errors.
If everything is correct function calls functions for generating XML
*/
function parse($words, $words_count, $dom, $program)
{
    global $comments;
    global $loc;
    global $jumps;
    global $labels;
    global $labels_control;

    $instruction_name = strtoupper($words[0]);

    //Main body of parse function
    switch($instruction_name)
    {
        //zero operand
        case 'CREATEFRAME':
        case 'PUSHFRAME':
        case 'POPFRAME':
        case 'RETURN':
        case 'BREAK':
            if($words_count != 1)
            {
                fwrite(STDERR, "Wrong number of instruction arguments - syntax error!\n");
                exit(23);
            }
            if($instruction_name === "RETURN")
            {
                $jumps += 1;
            }
            $loc += 1;
            generate_instruction($dom, $program, $instruction_name);
        break;

        //one operand
        case 'DEFVAR':
        case 'POPS':
            $loc += 1;
            if($words_count > 2 || $words_count == 1)
            {
                fwrite(STDERR, "Wrong number of instruction arguments - syntax error!\n");
                exit(23);
            }
            //var
            if(check_var($words[1]))
            {
                generate_instruction($dom, $program, $instruction_name);
                generate_argument($dom, "var", "arg1", $words[1]);
            }
            else
            {
                fwrite(STDERR, "Syntax error!\n");
                exit(23);
            }
        break;
        case 'PUSHS':
        case 'EXIT':
        case 'DPRINT':
        case 'WRITE':
            $loc += 1;
            if($words_count > 2 || $words_count == 1)
            {
                fwrite(STDERR, "Wrong number of instruction arguments - syntax error!\n");
                exit(23);
            }
            //symb
            if(check_symb($words[1]))
            {
                generate_instruction($dom, $program, $instruction_name);
                $returned_array = generate_symb($words[1]);
                generate_argument($dom, $returned_array[0], "arg1", $returned_array[1]);
            }
            else
            {
                fwrite(STDERR, "Syntax error!\n");
                exit(23);
            }
        break;
        case 'CALL':
        case 'LABEL':
        case 'JUMP':
            $loc += 1;
            if($instruction_name === "LABEL")
            {
                if(array_search($words[1], $labels_control))
                {}
                else
                {
                    $labels += 1;
                    array_push($labels_control, $words[1]);
                }
            }
            else
            {
                $jumps += 1;
            }
            if($words_count > 2 || $words_count == 1)
            {
                fwrite(STDERR, "Wrong number of instruction arguments - syntax error!\n");
                exit(23);
            }
            //label
            if(check_label($words[1]))
            {
                generate_instruction($dom, $program, $instruction_name);
                generate_argument($dom, "label", "arg1", $words[1]);
            }
            else
            {
                fwrite(STDERR, "Syntax error!\n");
                exit(23);
            }
        break;
        //two operand
        case 'MOVE':
        case 'STRLEN':
        case 'TYPE':
        case 'NOT':
        case 'INT2CHAR':
            $loc += 1;
            if($words_count > 3 || $words_count <= 2)
            {
                fwrite(STDERR, "Wrong number of instruction arguments - syntax error!\n");
                exit(23);
            }
            //var, symb
            if(check_var($words[1]))
            {
                if(check_symb($words[2]))
                {
                    generate_instruction($dom, $program, $instruction_name);
                    generate_argument($dom, "var", "arg1", $words[1]);
                    $returned_array = generate_symb($words[2]);
                    generate_argument($dom, $returned_array[0], "arg2", $returned_array[1]);
                }
                else
                {
                    fwrite(STDERR, "Syntax error!\n");
                    exit(23);
                }
            }
            else
            {
                fwrite(STDERR, "Syntax error!\n");
                exit(23);
            }
        break;
        case 'READ':
            $loc += 1;
            if($words_count > 3 || $words_count <= 2)
            {
                fwrite(STDERR, "Wrong number of instruction arguments - syntax error!\n");
                exit(23);
            }
            //var, type
            if(check_var($words[1]))
            {
                if(check_type($words[2]))
                {
                    generate_instruction($dom, $program, $instruction_name);
                    generate_argument($dom, "var", "arg1", $words[1]);
                    generate_argument($dom, "type", "arg2", $words[2]);
                }
                else
                {
                    fwrite(STDERR, "Syntax error!\n");
                    exit(23);
                }
            }
            else
            {
                fwrite(STDERR, "Syntax error!\n");
                exit(23);
            }
        break;
        //three operand
        case 'ADD':
        case 'SUB':
        case 'MUL':
        case 'IDIV':

        case 'LT':
        case 'GT':
        case 'EQ':

        case 'AND':
        case 'OR':

        case 'STRI2INT':
        case 'CONCAT':
        case 'GETCHAR':
        case 'SETCHAR':
            $loc += 1;
            if($words_count > 4 || $words_count <= 3)
            {
                fwrite(STDERR, "Wrong number of instruction arguments - syntax error!\n");
                exit(23);
            }
            //var, symb, symb
            if(check_var($words[1]))
            {
                if(check_symb($words[2]))
                {
                    if(check_symb($words[3]))
                    {
                        generate_instruction($dom, $program, $instruction_name);
                        generate_argument($dom, "var", "arg1", $words[1]);
                        $returned_array = generate_symb($words[2]);
                        generate_argument($dom, $returned_array[0], "arg2", $returned_array[1]);
                        $returned_array = generate_symb($words[3]);
                        generate_argument($dom, $returned_array[0], "arg2", $returned_array[1]);
                    }
                    else
                    {
                        fwrite(STDERR, "Syntax error!\n");
                        exit(23);
                    }
                }
                else
                {
                    fwrite(STDERR, "Syntax error!\n");
                    exit(23);
                }
            }
            else
            {
                fwrite(STDERR, "Syntax error!\n");
                exit(23);
            }
        break;
        case 'JUMPIFEQ':
        case 'JUMPIFNEQ':
            $loc += 1;
            $jumps += 1;
            if($words_count > 4 || $words_count <= 3)
            {
                fwrite(STDERR, "Wrong number of instruction arguments - syntax error!\n");
                exit(23);
            }
            //label, symb, symb
            if(check_label($words[1]))
            {
                if(check_symb($words[2]))
                {
                    if(check_symb($words[3]))
                    {
                        generate_instruction($dom, $program, $instruction_name);
                        generate_argument($dom, "label", "arg1", $words[1]);
                        $returned_array = generate_symb($words[2]);
                        generate_argument($dom, $returned_array[0], "arg2", $returned_array[1]);
                        $returned_array = generate_symb($words[3]);
                        generate_argument($dom, $returned_array[0], "arg3", $returned_array[1]);
                    }
                    else
                    {
                        fwrite(STDERR, "Syntax error!\n");
                        exit(23);
                    }
                }
                else
                {
                    fwrite(STDERR, "Syntax error!\n");
                    exit(23);
                }
            }
            else
            {
                fwrite(STDERR, "Syntax error!\n");
                exit(23);
            }
        break;
        default:
        //Controls IPPcode20 header
        global $header;
        if(preg_match('/.IPPCODE20/', $instruction_name) === 1)
        {
            if($header == 1)
            {
                fwrite(STDERR, "Too much headers!\n");
                exit(21);
            }
            $header = 1;
        }
        elseif($words[0] == "")
        {
            break;
        }
        else
        {
            fwrite(STDERR, "Syntax error!\n");
            exit(22);
        }
        break;
    }
}

/*
-------------------
Start of script
-------------------
*/

/*
DECLARATIONS
*/
$id = 0;
$instruction;
$header = 0;

$statistics = false;
$stat_file;
$loc = 0;
$comments = 0;
$labels = 0;
$jumps = 0;
$labels_control = array();
array_push($labels_control, "DEBUG");

$dom = new DomDocument("1.0", "utf-8");
$program = $dom->createElement("program");
$program->setAttribute("language", "IPPcode20");
$dom->appendChild($program);

ini_set('display_errors', 'stderr');
/*
--------------
*/

//Checking input
$stats_to_print = check_arguments($argc, $argv);

/*
Main loop for controls all lines in file with IPPcode20
*/
while(!feof(STDIN))
{
    $line = fgets(STDIN);
    //Delete white spaces
    $line = preg_replace('/\t+/', ' ', $line);
    
    //Delete comments and increment for statistics
    if(preg_match('/#.*/', $line))
    {
        $comments += 1;
        $line = preg_replace('/#.*/', '', $line);
    }
    $words = explode(" ", trim($line));
    $words_count = sizeof($words);

    parse($words, $words_count, $dom, $program);
}

//Controls if header exists
if($header == 1)
{}
else
{
    fwrite(STDERR, "Missing header!\n");
    exit(21);
}

/*
Generated XML print to output
*/
$dom->formatOutput = true;
echo $dom->saveXML();

/*
If statistics are enabled - print them to statistics file
*/
if($statistics == True)
{
    $file = fopen($stat_file, "w");
    for($j = 0; $j < sizeof($stats_to_print); $j++)
    {
        if($stats_to_print[$j] === "loc")
        {
            fwrite($file, $loc);
            fwrite($file, "\n");
        }
        elseif($stats_to_print[$j] === "comments")
        {
            fwrite($file, $comments);
            fwrite($file, "\n");
        }
        elseif($stats_to_print[$j] === "jumps")
        {
            fwrite($file, $jumps);
            fwrite($file, "\n");
        }
        elseif($stats_to_print[$j] === "labels")
        {
            fwrite($file, $labels);
            fwrite($file, "\n");
        }
    }
    fclose($file);
}

exit(0);
?>