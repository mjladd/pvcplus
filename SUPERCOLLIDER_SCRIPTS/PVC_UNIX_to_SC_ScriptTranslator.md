# PVC UNIX to SuperCollider Script Translator

This SuperCollider program translates PVC Unix shell scripts into SuperCollider format. It parses shell scripts from the SCRIPTS directory and generates corresponding SC files in the SUPERCOLLIDER_SCRIPTS directory.

## Usage

1. Set the `scriptsDirectory` to point to your PVC SCRIPTS folder
2. Set the `superColliderDirectory` to point to your SUPERCOLLIDER_SCRIPTS folder
3. Uncomment the script names you want to translate in the array at the bottom
4. Execute the code in SuperCollider

## Source Code

```supercollider
(
var scriptsDirectory, shellScript, superColliderDirectory, inputFile, outputFile, lp="(", rp=")", lcb = "{", rcb = "}" ;

var openFiles = {
    var outputFileScriptName ;
    inputFile = File.new( scriptsDirectory ++ shellScript, "r" ) ;

    if( superColliderDirectory.isNil, {
        superColliderDirectory = scriptsDirectory ;
    });

    outputFileScriptName = superColliderDirectory ++ shellScript.replace( "S.",  "SC." ) ;
    outputFile = File.new( outputFileScriptName, "w" ) ;

} ;

var closeFiles = {

    a = rp ++ "\n" ; a.post ; outputFile.write( a ) ;
    inputFile.close ; outputFile.close ;
} ;



var compileScript = {

    var line, commandLines, routinesAndFiles, pvFlags,
        thisPvFlags, thisFlagDictionary, fileLines,
        lastLineOfUserSection, command, theseVariables,
        unixCommandCollect="" ;


    fileLines = List.new ;
    // GET COMMAND LINES
    commandLines = List.new ;
    while( { (line = inputFile.getLine(1024)).notNil }, {
        if(  ( line[0] != $# ) && ( line.contains( "=" ) ) && (line[0] != $- ), {
            for( 0, line.indexOf( $= ) - 1, {arg k;
                line[ k ] = line[ k ].toLower ;
            });
            t = false ; line = line.split( $= ) ;
            line[ 1 ].do({arg c; t = c.isAlpha || t  });
            if( t, {
                // STRING OR VARIABLE USING $
                if( line[1].includes( $$ ),{
                    line =  line[0] ++ "=" ++ line[1].replace( "$", "" ).toLower ;
                },{
                    // STRING
                    line =  line[0] ++ "=" ++ "\"" ++ line[1] ++ "\"" ;
                });
            },{
                // NUMBER
                line =  line[0] ++ "=" ++  line[1]  ;
            });
        });
        if(  line[0] == $- , {
            line = String.fill( line.size, {arg i;
                if( i > 2, {
                    line[ i ].toLower
                },{
                    line[ i ] ;
                });
            });
        });


        fileLines.add( line );
    });



    fileLines.do({arg line ;
        if( line.contains( "routine" ) && line.contains( "echo" ) && line.contains( "PVFLAGS" ), {
            line = line.replace( "echo","" ).replace( " ", "" ).replace( "\"", "" ).replace( ";", "" ).replace( "\\n", "" ) ;
            line = line.copyRange( 1, line.lastIndex ).split( $$ ) ;
            line = Array.fill( line.size, {arg i;
                String.fill( line[ i ].size, {arg k ;
                    line[ i ][ k ].toLower ;
                });
            });
            commandLines.add( line );
        });
    });


    // GET ROUTINES AND FILES
    routinesAndFiles = Array.new( commandLines.size ) ;
    commandLines.do({arg commandLine ;
        a = Array.fill( commandLine.size, {  0 }) ;
        commandLine.do({arg field, index ;
            if( field != "pvflags", {
                fileLines.do({arg line ;
                    if( line.contains(  field  ++ "="   ) && ( line[0] != $# ), {
                        z = line.copyRange(  line.indexOf( $= ) + 1, line.lastIndex );
                        a[ index ] = field -> z ;
                    });
                });
            });
        });
        b = List.new ;
        a.do({arg item; if( item.class == Association, { b.add( item ) } ) }) ;
        routinesAndFiles.add( b.asArray )
    });



    // GET PVFLAGS FOR ROUTINES
    pvFlags = Array.new( commandLines.size ) ;

    // MAKE PVFLAGS STRING FOR EACH COMMAND
    thisPvFlags = String.new( 8192 ) ;
    fileLines.do({arg line ;
        if( line.contains( "pvflags=" ), {
            thisPvFlags = "\\" ;
        },{
            if( thisPvFlags != "", {
                if( line.contains("\""), {
                    pvFlags.add( thisPvFlags ); thisPvFlags = "" ;
                },{
                    thisPvFlags = thisPvFlags ++ line ;
                });
            });
        });
    });

    pvFlags.do({arg flags, i ;
        // REPLACE BACKSLASH LINE CONTINUANT WITH SLASH.
        flags = flags.replace( " ", "" ).tr( $\\, $. ) ;
        // REPLACE ALL MULTIPLE SLASHES WITH SINGLE ONE.
        while( {flags.contains( ".." ) },{ flags = flags.replace( "..", "." ) });
        // MAKE ARRAY OF FLAG-LINES WITH SPLIT.
        flags = flags.split( $. ) ;
        // REMOVE THE FIRST AND LAST ARRAY ELEMENTS IF THEY CONTAIN QUOTE.
        if( flags[0].contains( "\"" ), { flags.removeAt( 0 ) }) ;
        if( flags[ flags.lastIndex ].contains( "\"" ), { flags.removeAt(  flags.lastIndex ) }) ;
        // PUT FLAGS INTO DICTIONARY WITH PRE-"$" AS KEY, AND POST "$" AS VALUE.
        thisFlagDictionary = IdentityDictionary.new ;
        flags.do({arg flag ;
            // SPLIT FLAG FROM VARIABLE
            z = flag.split( $$ );
            // IF NO VARIABLE (I.E. DIRECT SETTING OF FLAG) AND HENCE NO SPLIT,
            // THEN ADD nil VALUE TO ARRAY AS VALUE.
            if( z.size == 1, {
                z = [ z[ 0 ], " " ] ;
            });
            if( z[0][0] == $-, {
                thisFlagDictionary.add( z[0].asSymbol -> z[1] ) ;
            });
        });
        pvFlags[ i ] = thisFlagDictionary ;
    });


    "" ;

    "" ;

    // GLOBAL VARIABLES AND FUNCTIONS
    // FUNCTION NAME

    a = "var commands=\"\" ;\n\n" ; a.post ; outputFile.write( a ) ;




    a = "var " ; a.post ; outputFile.write( a ) ;
    a = shellScript.replace( "S.", "" )  ++ " = " ++ lcb ++ " arg iterations=1 ; \n" ; a.post ; outputFile.write( a ) ;
    // DECLARE FUNCTION VARIABLES
    a = "\n\tvar temp,\n\t\tdataString, \n\t\tcommandLines=\"\"" ; a.post ; outputFile.write( a ) ;

    // PRINT ROUTINE, FILE, AND FLAG VARIABLES IN VARIABLE-SETTING INTERNAL FUNCTION.
    // USE SET TO NOT DOUBLE PRINT VARIABLES USED FOR MULTIPLE ROUTINES.
    theseVariables = List.new ;

    routinesAndFiles.do({arg routineAndFiles ;

        if( routineAndFiles.size >= 2, {
            for( 1, routineAndFiles.lastIndex, {arg i ;
                    theseVariables.add( routineAndFiles[ i ].key ) ;
            });
        });
    });

    pvFlags.do({arg thisFlagDictionary, i ;
        thisFlagDictionary.do({arg assoc ;
            if( (assoc != " "), {
                theseVariables.add( assoc ) ;
            });
        });
    });

    // PRINT OUT VARIABLES
    theseVariables.asSet.do({arg variable ;
        a = ", \n" ; a.post ; outputFile.write( a ) ;
        a = "\t\t" ; a.post ; outputFile.write( a ) ;
        a = variable ; a.post ; outputFile.write( a );
    });




    a = "\n\t; \n" ; a.post ; outputFile.write( a ) ;


    // FIND "OFFICE USE ONLY" LINE
    fileLines.do({arg line, i;
        if( line.contains( "OFFICE USE ONLY" ), {
            lastLineOfUserSection = i - 1 ;
        });
    });

    // INTERNAL FUNCTION FOR SETTING ROUTINE VARIABLES
    a = "\t\t// *** INTERNAL FUNCTION FOR SETTING ROUTINE VARIABLES ***\n" ; a.post ; outputFile.write( a ) ;
    a = "\n\tvar setRoutineVariables = " ++ lcb ++ "arg iteration=0 ;  \n\n" ; a.post ; outputFile.write( a );
    a = ( " \t\t//" ++ String.fill( 75, {Char.bullet} ) ++ "\n" ) ; a.post ; outputFile.write( a ) ;

    fileLines.copyRange(1, lastLineOfUserSection ).do({arg line;

        // WRITE COMMAND FROM UNIX COMMAND COLLECTOR IF NOT EMPTY AND
        // NEXT LINE IS VARIABLE ASSIGNMENT OR COMMENT.
        if( (line[0] == $#) || (line.contains( "=" )), {

            // DUMP UNIX COMMAND
            if( unixCommandCollect.isEmpty.not, {
                a = ("\n\t\taddCommand.value" ++ lp ++ "\n") ; a.post ; outputFile.write( a ) ;
                a = ("\t\t\t\"" ++ unixCommandCollect ++ "\"\n") ; a.post ; outputFile.write( a ) ;
                a = ("\t\t" ++ rp ++ " ;\n\n") ; a.post ; outputFile.write( a ) ;
                unixCommandCollect = "" ;
            });
        });


        if( line[0] == $#, {
            // COMMENT LINES "#"
            if( line.findAll( lp ).size < line.findAll( rp ).size, {
                //
                i = line.findAll( rp )[0] ; //
                if( line[i - 1].isSpace.not && (i != line.lastIndex) && (line.copyRange( i, line.lastIndex ).replace( " ", "" ) != rp ), {
                    line[     //
                        line.findAll( rp )[0]         ] = $: ;
                });
            });
            line = "\t\t//" ++ line.copyRange( 1, line.lastIndex ) ;
            while( { (l = line.findAll( lp ).size) != (r = line.findAll( rp ).size) },{
                if( l > r, {
                    t = line.split( lp[0] ) ; line = "" ;
                    t.size.do({arg i;
                        if ( (i != 0) && (i != t.lastIndex ), {
                            line = line ++ lp ;
                        });
                        line = line ++ t[ i ] ;
                    });
                },{
                    t = line.split( rp[0] ) ; line = "" ;
                    t.size.do({arg i;
                        if ( (i != (t.lastIndex - 1) ) && (i != t.lastIndex ), {
                            line = line ++ rp ;
                        });
                        line = line ++ t[ i ] ;
                    });
                });

            });
            a = line ++ "\n" ; a.post ; outputFile.write( a ) ;
        },{
            // NON-COMMENT LINES
            if( line.contains( "=" ), {
                // VARIABLE SETTING
                line = line.replace( " ", "" ) ;
                line = line.split( $= ) ;
                if( line[1][0] == $., {  line[1] = "0" ++ line[1] }) ;
                if( line[1].last == $., {  line[1] = line[1] ++ "0" }) ;
                a = ( "\t\t" ++ line[0] ++ " = " ++ line[1] ++  " ;\n") ; a.post ; outputFile.write( a ) ;        },{
                if( line.replace( " ", "" ).isEmpty.not, {
                    unixCommandCollect = unixCommandCollect ++ line.replace( "\\", " \n\t\t\t" ) ;

                    if( line.contains( ">" ) && (line.replace( " ", "" ).split( $> ).last.isEmpty.not), {                        a = ("\n\t\taddCommand.value(\n") ; a.post ; outputFile.write( a ) ;
                        a = ("\t\t\t\"" ++ unixCommandCollect ++ "\"\n") ; a.post ; outputFile.write( a ) ;
                        a = ("\t\t) ;\n") ; a.post ;  outputFile.write( a ) ;
                            unixCommandCollect = "" ;
                    });
                },{
                    a = "\n" ; a.post ; outputFile.write( a ) ;
                });
            });
        });


    });


    a = ( " \t\t//" ++ String.fill( 54, {Char.bullet} ) ++ "\n" ) ; a.post ; outputFile.write( a ) ;


    a = "\n\t" ++ rcb ++ " ;\n" ; a.post ; outputFile.write( a ) ;

    a = "\t//*********************************************************************************************************\n" ; a.post ; outputFile.write( a ) ;
    a = "\t//************* COMMAND LINE SETUP -- OFFICE USE ONLY ***************************************\n" ; a.post ; outputFile.write( a ) ;
    a = "\t//************* DO NOT CHANGE ANYTHING IN THE FUNCTION BELOW THIS LINE ***************\n" ; a.post ; outputFile.write( a ) ;
    a = "\t//*********************************************************************************************************\n" ; a.post ; outputFile.write( a ) ;
    a = "\n" ; a.post ; outputFile.write( a ) ;
    a = "\titerations.do" ++ lp ++ lcb ++ "arg n; \n" ; a.post ; outputFile.write( a ) ;
    a = ("\t\tsetRoutineVariables.value" ++ lp ++ " iteration: n " ++ rp ++ " ; \n") ; a.post ; outputFile.write( a ) ;
    // MAKE EACH UNIX COMMAND
        routinesAndFiles.do({arg thisRoutineAndFiles, k ;
            command = "" ;
            a = ("\n\t\t// ******** UNIX COMMAND LINE " ++ k ++ " ***********\n") ; a.post ; outputFile.write( a ) ;
            a = ("\t\taddCommand.value(\n") ; a.post ; outputFile.write( a ) ;

            a = "\n\t\t\t\t\t// * ROUTINE * \n" ; a.post ; outputFile.write( a ) ;
            command = command ++ thisRoutineAndFiles[0].value ++ (" ++ \" \" ++") ;
            a = ("\t\t\t" ++ command ++ "\n" ) ; a.post ; outputFile.write( a ) ;
            command = "" ;

            a = "\n\t\t\t\t// * FLAGS * \n" ; a.post ; outputFile.write( a );
            pvFlags[ k ].keys.do({arg key, j ;
                command = command ++ (" \" ") ++ key.asSymbol ++ ("\" ++ ")  ++
                if( pvFlags[ k ][ key ] == " ", {
                    ""
                },{
                    if( j < (pvFlags[ k ].keys.size - 1), {
                         pvFlags[ k ][ key ] ++ (" ++ ");
                    },{
                         pvFlags[ k ][ key ];
                    });
                }) ;
                if( command.size >= 75 , {
                    a = ("\t\t\t" ++ command ++ "\n" ) ; a.post ; outputFile.write( a ) ;
                    command = "" ;
                });
            });
            if( command.size > 0, {
                a = ("\t\t\t" ++ command ++ "\n") ; a.post ; outputFile.write( a ) ;
                command = "" ;
            });



            if( thisRoutineAndFiles.size >= 2, {

                a = "\n\t\t\t\t// * INPUT/OUTPUT FILES * \n" ; a.post ; outputFile.write( a );

                command = command ++ (" ++ ");
                for( 1, thisRoutineAndFiles.lastIndex, {arg m;
                    command = command ++ (" \" \"  ++ ") ++ thisRoutineAndFiles[m].key ;
                    if( m < thisRoutineAndFiles.lastIndex, {
                        command = command ++ (" ++ \"  \" ++ ") ;
                    });
                }) ;
            });

            a = ("\t\t\t" ++ command ++ "\n") ; a.post ; outputFile.write( a );
            a = ("\n\t\t) ;\n \n" ) ; a.post ; outputFile.write( a );
            a = "\n" ; a.post ; outputFile.write( a ) ;
        });

        // ADD ROUTINE TO STRING OF UNIX COMMANDS
        a = "\n" ; a.post ; outputFile.write( a ) ;
    a = "\t" ++ rcb ++ rp ++ " ;\n" ; a.post ; outputFile.write( a ) ;



    a = rcb ++ " ;\n" ; a.post ; outputFile.write( a ) ;



    a = "\n\n\n" ; a.post ; outputFile.write( a ) ;

    a = "/////////////////////////////////////////////////////////////////////////////////////\n" ; a.post ; outputFile.write( a ) ;
    a = "//         ---------- FUNCTION CALLS  ----------            \n" ; a.post ; outputFile.write( a ) ;
    a = "/////////////////////////////////////////////////////////////////////////////////////\n" ; a.post ; outputFile.write( a ) ;

    a = "\n\n// *** CLEAR SCREEN ***\n" ; a.post ; outputFile.write( a ) ;
    a = "Document.listener.string=\"\"; \n\n" ; a.post ; outputFile.write( a ) ;


    a = ( shellScript.replace( "S.", "" ) ++ ".value( iterations: 1 ) ; \n") ; a.post ; outputFile.write( a ) ;

    a = "\n" ; a.post ; outputFile.write( a ) ;


    a = ("runCommands.value ;\n\n\n") ; a.post ; outputFile.write( a ) ;


    a = ("\"..........DONE!\" \n") ; a.post ; outputFile.write( a ) ;


    "" ;

} ;


// MAKE ADD AND RUN COMMANDS FUNCTIONS

var makeCommandFunctions = {


    a = (lp ++ "\n") ; a.post ; outputFile.write( a ) ;

        // addCommand
    a = "\nvar addCommand = " ++ lcb ++ " arg command ;\n" ; a.post ; outputFile.write( a ) ;
    a ="\tcommand = command.replace" ++ lp ++ " \"\\n\", \" \" " ++ rp ++ ".replace" ++ lp ++ " \"\\t\", \" \" " ++ rp ++ " ;  \n" ;
            a.post ; outputFile.write( a ) ;
    a = "\tcommands = commands ++ command ++ \" \\n\" ; \n" ; a.post ; outputFile.write( a );

    a = rcb ++ " ; \n\n" ; a.post ; outputFile.write( a ) ;

        //  runCommand
    a = "\n\nvar runCommands = " ++ lcb ++ "\n" ; a.post ; outputFile.write( a ) ;
    a = "\tvar homeDirectory ; \n" ; a.post ; outputFile.write( a );
    a = "\thomeDirectory =  \"HOME\".getenv ;\n" ; a.post ; outputFile.write( a );
    a = "\tcommands = commands.replace( \"~/\", homeDirectory ++ \"/\" ) ;\n" ; a.post ; outputFile.write( a ) ;
    a = "\tcommands.replace(  \"\\n\", \" ;\\n\\n\" ).postln  ; \n" ; a.post ; outputFile.write( a ) ;
    a = "\tcommands.runInTerminal ; \n" ; a.post ; outputFile.write( a ) ;
    a = rcb ++ " ; \n\n" ; a.post ; outputFile.write( a ) ;

} ;

// CLEAR SCREEN
Document.listener.string="" ;


// Set these paths to match your installation
scriptsDirectory =  "/path/to/PVCplus/SCRIPTS/" ;
superColliderDirectory = "/path/to/PVCplus/SUPERCOLLIDER_SCRIPTS/" ;

[
// Uncomment the scripts you want to translate:
//"S.irconvolvesequencer_with_roomresponsesequencer",
//"S.irconvolvesequencer",
//"S.roomresponsesequencer",
//"S.centroid",
//"S.channelmix",
//"S.chordmapperplus",
//"S.compander",
//"S.convolver",
//"S.delayfilter_with_groupdelaymaker",
//"S.envelope",
//"S.filtdeviator_with_analysis",
//"S.filtdeviator_with_breakpoint_synthesis",
//"S.filtdeviator_with_chord_synthesis",
//"S.filter_with_analysis",
//"S.filter_with_breakpoint_synthesis",
//"S.filter_with_chord_synthesis",
//"S.fluxoid",
//"S.formantsmapper",
//"S.freqresponse",
//"S.harmonizer",
//"S.impulseresponse",
//"S.inharmonator",
//"S.irconvolver",
//"S.noisefilter",
//"S.peakformant",
//"S.pitchtracker",
//"S.plainpv",
//"S.pvanalysis",
//"S.ratechanger",
//"S.ring",
//"S.ringfilter_with_analysis",
//"S.ringfilter_with_breakpoint_synthesis",
//"S.ringfilter_with_chord_synthesis",
//"S.ringtvfilter",
//"S.roomresponsemaker",
//"S.specflattracker",
//"S.spectralextractor",
//"S.spectwarper",
//"S.tvfiltdeviator",
//"S.tvfilter",
//"S.twarp"
].do({arg script ;
    shellScript = script ;
    ("shellScript: " ++ shellScript).postln ;
    openFiles.value ;
    makeCommandFunctions.value ;
    compileScript.value ;
    closeFiles.value ;

});

)
```

## Available Scripts

The translator can convert the following PVC shell scripts to SuperCollider format:

- `S.centroid`
- `S.channelmix`
- `S.chordmapperplus`
- `S.compander`
- `S.convolver`
- `S.delayfilter_with_groupdelaymaker`
- `S.envelope`
- `S.filtdeviator_with_analysis`
- `S.filtdeviator_with_breakpoint_synthesis`
- `S.filtdeviator_with_chord_synthesis`
- `S.filter_with_analysis`
- `S.filter_with_breakpoint_synthesis`
- `S.filter_with_chord_synthesis`
- `S.fluxoid`
- `S.formantsmapper`
- `S.freqresponse`
- `S.harmonizer`
- `S.impulseresponse`
- `S.inharmonator`
- `S.irconvolver`
- `S.irconvolvesequencer`
- `S.irconvolvesequencer_with_roomresponsesequencer`
- `S.noisefilter`
- `S.peakformant`
- `S.pitchtracker`
- `S.plainpv`
- `S.pvanalysis`
- `S.ratechanger`
- `S.ring`
- `S.ringfilter_with_analysis`
- `S.ringfilter_with_breakpoint_synthesis`
- `S.ringfilter_with_chord_synthesis`
- `S.ringtvfilter`
- `S.roomresponsemaker`
- `S.roomresponsesequencer`
- `S.specflattracker`
- `S.spectralextractor`
- `S.spectwarper`
- `S.tvfiltdeviator`
- `S.tvfilter`
- `S.twarp`
