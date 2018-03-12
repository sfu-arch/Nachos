#!/usr/bin/python   

import sys
import gzip
import argparse

histogram = {} 
# not_conflict, conflict
histogram['LS'] = [0,0]
histogram['SL'] = [0,0]
histogram['SS'] = [0,0]

ROB_SIZE = 32
ITERATION_SIZE = 100
BYTE_WIDTH = 8 
# x = range(1,10)
# y = range(8,20)
# print list(set(x) & set(y))





#--------------------------------------------------------------------------------
def print_histogram():
    print  "SS " \
            + str(histogram['SS'][0]) + " " \
            + str(histogram['SS'][1]) + " " \
            + "LS " \
            + str(histogram['LS'][0]) + " " \
            + str(histogram['LS'][1]) + " " \
            + "SL " \
            + str(histogram['SL'][0]) + " " + str(histogram['SL'][1]) 

#--------------------------------------------------------------------------------
def check_conflict(entry1,entry2):
        addr1 = range(entry1[1],entry1[2])
        addr2 = range(entry2[1],entry2[2])
        return  bool(set(addr1) & set(addr2))

def check_conflict_0(entry1,entry2):
        S1 = entry1[1]
        E1 = entry1[2]
        S2 = entry2[1]
        E2 = entry2[2]

        # print   "S1: " + str(S1)  + \
        #         " E1: " + str(E1)  + \
        #         " S2: " + str(S2)  + \
        #         " E2: " + str(E2)  

        if(E1 <= S2) or (E2 <= S1):
            return False
        else:
            return True



#--------------------------------------------------------------------------------
def calculate_mem_dep(rob):
    print "rob_length ------------"
    print len(rob)
    for idx1, entry1 in enumerate(rob[:-1]):
        for idx2, entry2 in enumerate(rob[idx1+1:]):
            is_conflict = check_conflict_0(entry1, entry2)
            # print str(entry1[0]) + "----------" + str(entry2[0]) + "  is_conflict: "+ str(is_conflict) 

            if entry1[0] == 0 and entry2[0] == 1:
                histogram['LS'][is_conflict] +=1
     
            elif entry1[0] == 1 and entry2[0] == 0:
                # print "SL: " + str(is_conflict)
                histogram['SL'][is_conflict] +=1

            elif entry1[0] == 1 and entry2[0] == 1:
                # print "SS: " + str(is_conflict)
                histogram['SS'][is_conflict] +=1







#--------------------------------------------------------------------------------
def main(addrfile, is_fixed_byte):
    rob = []
    index = 0
    iteration_no =0

    with gzip.open(addrfile, 'r') as f:
        line = f.read()
        # print line
        # for line in f.read().splitlines():
        perline  = line.split("\n")
        del perline[-1]
        # print perline

        for sub_string in perline :
            sub_data = sub_string.split(" ")
            if sub_data:
                # print sub_data
                # sub_data = perline.split(" ") 
                # print sub_data
                is_write = 0
                if sub_data[0] == 'R':
                    is_write = 0
                else:
                    is_write = 1

                #converting to hex
                start_addr = int(sub_data[1], 16)
                if( is_fixed_byte == False):
                    # print "False:"
                    end_addr = start_addr + int(sub_data[2]) 
                else:
                    end_addr = start_addr + BYTE_WIDTH 
                    # print "True:"
                
                # print str(is_write) + " " + str(start_addr) + " " + str(end_addr)

                entry = []
                entry = [is_write, start_addr,end_addr]
                rob.append(entry)
                index +=1
                # print rob

                if(index == ROB_SIZE):
                    iteration_no +=1
                    # print "iteration_no: " + str(iteration_no)
                    index = 0
                    calculate_mem_dep(rob)
                    rob = []
                    # print_histogram()

                if(iteration_no >= ITERATION_SIZE):
                    break
                    # print histogram
                    # print_histogram()

                    exit()
    calculate_mem_dep(rob)
    rob = []
    print_histogram()



#--------------------------------------------------------------------------------
if __name__ == "__main__":
    # main(sys.argv[1], sys.argv[2])
    parser = argparse.ArgumentParser()
    parser.add_argument("addrfile")
    parser.add_argument('--fixed_byte',
            action='store_true',
            help='verbose flag' )
    args = parser.parse_args()
    b = False
    if(args.fixed_byte):
        b= True

    # print "ARG2: " + str(b)
    main(args.addrfile, b)


