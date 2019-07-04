import csv

program = [-1] * 128 * 8
entries = [0] * 128 * 8

with open('program_2019_final.csv', newline='') as csvfile:
    spamreader = csv.DictReader(csvfile, dialect='excel')
    count = 1
    duplicates = 0;
    missing_data = 0;
    for row in spamreader:
        count = count + 1

        if row['Ignition Event Time'] != '':
            ignition_time = int(float(row['Ignition Event Time']) * 1000)
        else:
            print("Row", count, " - Missing ignition time")
            missing_data = missing_data + 1
            continue

        if row['Module Address'] != '':
            addr_module = int(row['Module Address'])
        else:
            print("Row", count, " - Missing module address")
            missing_data = missing_data + 1
            continue

        if row['Slat Address'] != '':
            addr_slat = int(row['Slat Address'])
        else:
            print("Row", count, " - Missing slat address")
            missing_data = missing_data + 1
            continue

        if row['Pin Address'] != '':
            addr_pin = int(row['Pin Address'])
        else:
            print("Row", count, " - Missing pin address")
            missing_data = missing_data + 1
            continue

        #print("Row", count, "added %s at %u, %u, %u" % (row['Effect Name'], addr_module, addr_slat, addr_pin))

        index = addr_module*128 + addr_slat*16 + addr_pin

        if program[index] == -1:
            program[index] = ignition_time
            entries[index] = count
        elif program[index] == ignition_time:
            print(end='')
        else:
            print("Row", count, " - Duplicate with row", entries[index], "(differerent firing times", program[index]/1000, "vs", ignition_time/1000, "): ", addr_module, ',', addr_slat, ',', addr_pin)
            duplicates = duplicates + 1


    added = 0
    print("")
    print("")
    print("/*  Module 0  ,   Module 1  ,   Module 2  ,   Module 3  ,   Module 4  ,   Module 5   */")
    for socket in range(0, 8):
        for cue in range(0, 16):
            print("{ ", end='')
            for remote in range(0,6):
                index = remote*128 + socket*16 + cue
                val = program[index]
                if val != -1:
                    print("%12u" % (program[index]), end='')
                    added = added + 1
                else:
                    print("(uint32_t)-1", end='')

                if remote != 5:
                    print(", ", end='')
            print(" }", end='')
            if socket == 7 and cue == 15:
                print("", end='')
            else:
                print(",", end='')
            print(" /* Socket %u Cue %u */" % (socket, cue))
    print("")    
    print("")
    print(duplicates, "duplicate cues with different firing times.")
    print(missing_data, "cues found with missing firing data.")
    print(added, "cues added to program.")

    print("")
    print("")
    print("Expected scan response:")
    for remote in range(0,6):
        print("Remote-%d scan response:" % remote)
        for socket in range(0,8):
            print("Socket %d: " % socket, end='')
            for cue in range(0,16):
                index = remote*128 + socket*16 + cue
                if program[index] == -1:
                    print("    ", end='')
                else:
                    print("Q%02d " % (cue), end='')
            print("")
