# python3 program
def gen(i):
    s = 'VECTOR'
    num = str(hex(int(i)))
    num = '0' + num[1:]
    if len(num) == 3:
        num = num[:2] + '0' + num[2:]
    s += ' '
    s += num
    s += ', '
    if num == '0x1e':
        s += 'ERROR_CODE'
    else:
         s += 'ZERO'
    return s

for i in range(0, 33):
    print(gen(i))
