<?php declare(strict_types = 1);

class Tester
{
    public function testInputArguments()
    {
        $wrongArgument1 = shell_exec('./isabot -hhg');
        if ($wrongArgument1 === "ERROR: -1 : Wrong arguments!\n") {
            echo "TEST 1 was successfull.\n";
        } else {
            echo "TEST 1 was unsuccessfull!\n";
        }

        $wrongArgument2 = shell_exec('./isabot');
        if ($wrongArgument2 === "ERROR: -1 : Too less arguments! Try -h or -help\n") {
            echo "TEST 2 was successfull.\n";
        } else {
            echo "TEST 2 was unsuccessfull!\n";
        }

        $wrongArgument3 = shell_exec('./isabot -g -blabla -f -t -rrer asdp');
        if ($wrongArgument3 === "ERROR: -1 : Too many arguments! Try -h or -help\n") {
            echo "TEST 3 was successfull.\n";
        } else {
            echo "TEST 3 was unsuccessfull!\n";
        }

        $showHelp1 = shell_exec('./isabot -h');
        if ($showHelp1 === "ISA-BOT NAPOVEDA:\nPovolene argumenty ke spousteni:\n[-h|-help] - Pro vypis napovedy\n[-v|-verbose] - Pro vypis zprav na ktere BOT reaguje na STDOUT\n-t <bot_access_token> - Pro zadani pristupoveho tokenu BOTa. Tento argument je povinny\n") {
            echo "TEST 4 was successfull.\n";
        } else {
            echo "TEST 4 was unsuccessfull!\n";
        }

        $wrongBotToken = shell_exec('./isabot -t -v');
        if ($wrongBotToken === "ERROR: -5 : Wrong BOT Token!\n") {
            echo "TEST 5 was successfull.\n";
        } else {
            echo "TEST 5 was unsuccessfull!\n";
        }

        $missingBotToken = shell_exec('./isabot -t');
        if ($missingBotToken === "ERROR: -1 : Mising BOT token\n") {
            echo "TEST 6 was successfull.\n";
        } else {
            echo "TEST 6 was unsuccessfull!\n";
        }

        $showHelp2 = shell_exec('./isabot -t -h -v');
        if ($showHelp1 === "ISA-BOT NAPOVEDA:\nPovolene argumenty ke spousteni:\n[-h|-help] - Pro vypis napovedy\n[-v|-verbose] - Pro vypis zprav na ktere BOT reaguje na STDOUT\n-t <bot_access_token> - Pro zadani pristupoveho tokenu BOTa. Tento argument je povinny\n") {
            echo "TEST 7 was successfull.\n";
        } else {
            echo "TEST 7 was unsuccessfull!\n";
        }
    }
}

$tester = new Tester();
$tester->testInputArguments();