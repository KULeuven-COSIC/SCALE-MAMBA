#!/usr/bin/env python


#     ===== Compiler usage instructions =====
# 
# See documentation for details on the Compiler package


from optparse import OptionParser
import Compiler

def main():
    usage = "usage: %prog [options] filename [args]"
    parser = OptionParser(usage=usage)
    parser.add_option("-n", "--nomerge",
                      action="store_false", dest="merge_opens", default=True,
                      help="don't attempt to merge open instructions")
    parser.add_option("-o", "--output", dest="outfile",
                      help="specify output file")
    parser.add_option("-a", "--asm-output", dest="asmoutfile",
                      help="asm output file for debugging")
    parser.add_option("-d", "--debug", action="store_true", dest="debug",
                      help="keep track of trace for debugging")
    parser.add_option("-c", "--comparison", dest="comparison", default="log",
                      help="comparison variant: log|plain")
    parser.add_option("-D", "--dead-code-elimination", action="store_true",
                      dest="dead_code_elimination", default=False,
                      help="eliminate instructions with unused result")
    parser.add_option("-r", "--noreorder", dest="reorder_between_opens",
                      action="store_false", default=True,
                      help="don't attempt to place instructions between start/stop opens")
    parser.add_option("-M", "--preserve-mem-order", action="store_true",
                      dest="preserve_mem_order", default=False,
                      help="preserve order of memory instructions; possible efficiency loss")
    parser.add_option("-u", "--noreallocate", action="store_true", dest="noreallocate",
                      default=False, help="don't reallocate")
    parser.add_option("-m", "--max-parallel-open", dest="max_parallel_open",
                      default=False, help="restrict number of parallel opens")
    parser.add_option("-P", "--profile", action="store_true", dest="profile",
                      help="profile compilation")
    parser.add_option("-C", "--continous", action="store_true", dest="continuous",
                      help="continuous computation")
    parser.add_option("-s", "--stop", action="store_true", dest="stop",
                      help="stop on register errors")

    parser.add_option("-f", "--fdflag", action="store_false",
                      dest="fdflag", default=True,
                      help="de-activates under-over flow check for sfloats")
    options,args = parser.parse_args()

    if len(args) < 1:
        parser.print_help()
        return

    def compilation():
        prog = Compiler.run(args, options, 
                            merge_opens=options.merge_opens, 
                            debug=options.debug)
        prog.write_bytes(options.outfile)
        if options.asmoutfile:
            for tape in prog.tapes:
                tape.write_str(options.asmoutfile + '-' + tape.name)

    if options.profile:
        import cProfile
        p = cProfile.Profile().runctx('compilation()', globals(), locals())
        p.dump_stats(args[0] + '.prof')
        p.print_stats(2)
    else:
        compilation()

if __name__ == '__main__':
    main()
