


MH-CHART(1)         UNIX Programmer's Manual          MH-CHART(1)



NAME
     mh-chart - Chart of _M_H Commands

SYNOPSIS
     ali [-alias aliasfile] [-list] [-nolist] [-normalize]
          [-nonormalize] [-user] [-nouser] names ...  [-help]

     anno [+folder] [msgs] [-component field] [-inplace]
          [-noinplace] [-text body] [-help]

     burst [+folder] [msgs] [-inplace] [-noinplace] [-quiet]
          [-noquiet] [-verbose] [-noverbose] [-help]

     comp [+folder] [msg] [-draftfolder +folder]
          [-draftmessage msg] [-nodraftfolder] [-editor editor]
          [-noedit] [-file file] [-form formfile] [-use] [-nouse]
          [-whatnowproc program] [-nowhatnowproc] [-help]

     dist [+folder] [msg] [-annotate] [-noannotate]
          [-draftfolder +folder] [-draftmessage msg]
          [-nodraftfolder] [-editor editor] [-noedit]
          [-form formfile] [-inplace] [-noinplace]
          [-whatnowproc program] [-nowhatnowproc] [-help]

     folder [+folder] [msg] [-all] [-fast] [-nofast] [-header]
          [-noheader] [-pack] [-nopack] [-recurse] [-norecurse]
          [-total] [-nototal] [-print] [-noprint] [-list]
          [-nolist] [-push] [-pop] [-help]

     folders

     forw [+folder] [msgs] [-annotate] [-noannotate]
          [-draftfolder +folder] [-draftmessage msg]
          [-nodraftfolder] [-editor editor] [-noedit]
          [-filter filterfile] [-form formfile] [-format]
          [-noformat] [-inplace] [-noinplace]
          [-whatnowproc program] [-nowhatnowproc] [-help]

     forw [+folder] [msgs] [-digest list] [-issue number]
          [-volume number] [other switches for _f_o_r_w] [-help]















Printed 1/10/87                MH                               1






MH-CHART(1)         UNIX Programmer's Manual          MH-CHART(1)



     inc [+folder] [-audit audit-file] [-noaudit] [-changecur]
          [-nochangecur] [-file name] [-form formatfile]
          [-format string] [-silent] [-nosilent] [-truncate]
          [-notruncate] [-width columns] [-host host]
          [-user user] [-pack file] [-nopack] [-rpop] [-norpop]
          [-help]

     mark [+folder] [msgs] [-sequence name ...] [-add] [-delete]
          [-list] [-public] [-nopublic] [-zero] [-nozero] [-help]

     /usr/misc/mh/lib/mhl [-bell] [-nobell] [-clear] [-noclear]
          [-folder +folder] [-form formfile] [-length lines]
          [-width columns] [-moreproc program] [-nomoreproc]
          [files ...] [-help]

     mhmail [ addrs ... [-body text] [-cc addrs ...] [-from addr]
          [-subject subject]] [-help]

     mhpath [+folder] [msgs] [-help]

     msgchk [users ...] [-host host] [-user user] [-rpop]
          [-norpop] [-help]

     msh [-prompt string] [-scan] [-noscan] [-topcur] [-notopcur]
          [file] [-help]

     next [+folder] [-header] [-noheader] [-showproc program]
          [-noshowproc] [switches for _s_h_o_w_p_r_o_c] [-help]

     packf [+folder] [msgs] [-file name] [-help]

     pick [+folder] [msgs] [-and ...] [-or ...] [-not ...]
          [-lbrace ... -rbrace] [--component pattern]
          [-after date] [-before date] [-datefield field]
          [-sequence name ...] [-public] [-nopublic] [-zero]
          [-nozero] [-list] [-nolist] [-help]

     prev [+folder] [-header] [-noheader] [-showproc program]
          [-noshowproc] [switches for _s_h_o_w_p_r_o_c] [-help]

     prompter [-erase chr] [-kill chr] [-prepend] [-noprepend]
          [-rapid] [-norapid] file [-help]

     /usr/misc/mh/lib/rcvstore [+folder] [-create] [-nocreate]
          [-sequence name ...] [-public] [-nopublic] [-zero]
          [-nozero] [-help]









Printed 1/10/87                MH                               2






MH-CHART(1)         UNIX Programmer's Manual          MH-CHART(1)



     refile [msgs] [-draft] [-link] [-nolink] [-preserve]
          [-nopreserve] [-src +folder] [-file file] +folder ...
          [-help]

     repl [+folder] [msg] [-annotate] [-noannotate]
          [-cc all/to/cc/me] [-nocc all/to/cc/me]
          [-draftfolder +folder] [-draftmessage msg]
          [-nodraftfolder] [-editor editor] [-noedit]
          [-fcc +folder] [-filter filterfile] [-form formfile]
          [-format] [-noformat] [-inplace] [-noinplace] [-query]
          [-noquery] [-whatnowproc program] [-nowhatnowproc]
          [-width columns] [-help]

     rmf [+folder] [-interactive] [-nointeractive] [-help]

     rmm [+folder] [msgs] [-help]

     scan [+folder] [msgs] [-clear] [-noclear] [-form formatfile]
          [-format string] [-header] [-noheader] [-width columns]
          [-help]

     send [-alias aliasfile] [-draft] [-draftfolder +folder]
          [-draftmessage msg] [-nodraftfolder]
          [-filter filterfile] [-nofilter] [-format] [-noformat]
          [-forward] [-noforward] [-msgid] [-nomsgid] [-push]
          [-nopush] [-verbose] [-noverbose] [-watch] [-nowatch]
          [-width columns] [file ...] [-help]

     show [+folder] [msgs] [-draft] [-header] [-noheader]
          [-showproc program] [-noshowproc]
          [switches for _s_h_o_w_p_r_o_c] [-help]

     sortm [+folder] [msgs] [-datefield field] [-verbose]
          [-noverbose] [-help]

     vmh [-prompt string] [-vmhproc program] [-novmhproc]
          [switches for _v_m_h_p_r_o_c] [-help]

     whatnow [-draftfolder +folder] [-draftmessage msg]
          [-nodraftfolder] [-editor editor] [-noedit]
          [-prompt string] [file] [-help]

     whom [-alias aliasfile] [-check] [-nocheck] [-draft]
          [-draftfolder +folder] [-draftmessage msg]
          [-nodraftfolder] [file] [-help]










Printed 1/10/87                MH                               3






MH-CHART(1)         UNIX Programmer's Manual          MH-CHART(1)



     /usr/misc/mh/lib/ap [-form formatfile] [-format string]
          [-normalize] [-nonormalize] [-width columns] addrs ...
          [-help]

     /usr/misc/mh/lib/conflict [-mail name] [-search directory]
          [aliasfiles ...] [-help]

     /usr/misc/mh/lib/dp [-form formatfile] [-format string]
          [-width columns] dates ...  [-help]

     /usr/misc/mh/lib/install-mh [-auto] [-compat]

     /usr/misc/mh/lib/post [-alias aliasfile]
          [-filter filterfile] [-nofilter] [-format] [-noformat]
          [-msgid] [-nomsgid] [-verbose] [-noverbose] [-watch]
          [-nowatch] [-width columns] file [-help]







































Printed 1/10/87                MH                               4



