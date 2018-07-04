import csv

program = [-1] * 128 * 8

with open('Rhapsody in Blue with all positions added 2018 july 2 12 am.csv', newline='') as csvfile:
    spamreader = csv.DictReader(csvfile, dialect='excel')
    count = 1
    for row in spamreader:
        count = count + 1

        if row['Effect Name'] != '':
            name = row['Effect Name']
        else:
            print(count, " - Missing effect name")
            continue

        if row['Ignition Event Time'] != '':
            ignition_time = int(float(row['Ignition Event Time']) * 1000)
        else:
            print(count, " - Missing ignition time")
            continue

        if row['Module Address'] != '':
            addr_module = int(row['Module Address'])
        else:
            print(count, " - Missing module address")
            continue

        if row['Slat Address'] != '':
            addr_slat = int(row['Slat Address'])
        else:
            print(count, " - Missing slat address")
            continue

        if row['Pin Address'] != '':
            addr_pin = int(row['Pin Address'])
        else:
            print(count, " - Missing pin address")
            continue

        index = addr_module*128 + addr_slat*16 + addr_pin

        if program[index] == -1:
            program[index] = ignition_time
        elif program[index] == ignition_time:
            print()
        else:
            print(count, " - Duplicate with differing firing time: ", addr_module, ',', addr_slat, ',', addr_pin)

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