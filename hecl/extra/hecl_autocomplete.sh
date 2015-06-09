#!/bin/bash

_hecl ()
{
    local word=${COMP_WORDS[COMP_CWORD]}
    local filecmds=(init spec add remove group cook clean package)
    
    if [ $COMP_CWORD == 1 ]
    then
        COMPREPLY=($(compgen -W "${filecmds[*]} help" "${word}"))
        return 0
    elif [ $COMP_CWORD == 2 ]
    then
        case ${COMP_WORDS[1]} in
        init|add|remove|group|cook|clean|package)
            COMPREPLY=($(compgen -f -- "${word}"))
            ;;
        spec)
            COMPREPLY=($(compgen -W "enable disable" "${word}"))
            ;;
        help)
            COMPREPLY=($(compgen -W "${filecmds[*]}" "${word}"))
            ;;
        esac
    fi
}

complete -F _hecl hecl

