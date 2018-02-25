#!/usr/bin/python   

import sys
import gzip

histogram = {} 
# not_conflict, conflict
histogram['LL'] = [0,0]
histogram['LS'] = [0,0]
histogram['SL'] = [0,0]
histogram['SS'] = [0,0]

# ROB_SIZE = 128
# ITERATION_SIZE = 10000000
ITERATION_SIZE = 1
# BYTE_SIZE = 8
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
            + str(histogram['SL'][0]) + " " + str(histogram['SL'][1]) + " " \
            + "LL " \
            + str(histogram['LL'][0]) + " " + str(histogram['LL'][1]) 



#--------------------------------------------------------------------------------
# def check_conflict(entry1,entry2):
#         addr1 = range(entry1[1],entry1[2])
#         addr2 = range(entry2[1],entry2[2])
#         return  bool(set(addr1) & set(addr2))

def check_conflict(entry1,entry2):
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
    for idx1, entry1 in enumerate(rob[:-1]):
        for idx2, entry2 in enumerate(rob[idx1+1:]):
            # print str(entry1[0]) + "----------" + str(entry2[0])
            is_conflict = check_conflict(entry1, entry2)

            if entry1[0] == 0 and entry2[0] == 1:
                histogram['LS'][is_conflict] +=1
     
            elif entry1[0] == 1 and entry2[0] == 0:
                # print "SL: " + str(is_conflict)
                histogram['SL'][is_conflict] +=1

            elif entry1[0] == 1 and entry2[0] == 1:
                # print "SS: " + str(is_conflict)
                histogram['SS'][is_conflict] +=1


            elif entry1[0] == 0 and entry2[0] == 0:
                # print "LL: " + str(is_conflict)
                histogram['LL'][is_conflict] +=1




#--------------------------------------------------------------------------------
def main(addrfile):
    iter_addr_list = {}
    index = 0
    iteration_no =0

    with gzip.open(addrfile, 'r') as f:
        line = f.read()
        # print line
        perline  = line.split("\n")
        del perline[-1]

        for sub_string in perline :
            sub_data = sub_string.split(",")
            if sub_data:
                # print sub_data
                # sub_data = perline.split(" ") 
                # print sub_data
                iter_no = int(sub_data[0])
                is_write = 0
                if sub_data[1] == 'Store':
                    is_write = 1

                start_addr = int(sub_data[3])
                end_addr = start_addr + int(sub_data[4])

                # print "LD/ST: " + str(is_write)+ " Iter: " + str(iter_no) + " nodeid: " + str(sub_data[2]) + " start_Addr: " + str(start_addr) + " end_addR: " + str(end_addr)
                if iter_no in iter_addr_list:
                    listof_entries = iter_addr_list[iter_no]
                    listof_entries.append([is_write,start_addr,end_addr])
                else:
                    iter_addr_list[iter_no] = [[is_write,start_addr,end_addr]]

            # print "#iter: " + str(iteration_no)
            # iteration_no +=1
            # if(iteration_no == ITERATION_SIZE):
            #     break
            
            # print "#iter: " + str(len(iter_addr_list))
            if len(iter_addr_list) > ITERATION_SIZE :
                break
     
    for key, value in iter_addr_list.iteritems():
        # print len(value)
        calculate_mem_dep(value)

    # print iter_addr_list
    print_histogram()
#--------------------------------------------------------------------------------
if __name__ == "__main__":
    main(sys.argv[1])


