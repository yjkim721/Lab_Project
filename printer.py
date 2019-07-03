import matplotlib.pyplot as plt
import matplotlib as mpl

C_END     = "\033[0m"
C_BOLD    = "\033[1m"
C_INVERSE = "\033[7m"

C_BLACK  = "\033[30m"
C_RED    = "\033[31m"
C_GREEN  = "\033[32m"
C_YELLOW = "\033[33m"
C_BLUE   = "\033[34m"
C_PURPLE = "\033[35m"
C_CYAN   = "\033[36m"
C_WHITE  = "\033[37m"

C_BGBLACK  = "\033[40m"
C_BGRED    = "\033[41m"
C_BGGREEN  = "\033[42m"
C_BGYELLOW = "\033[43m"
C_BGBLUE   = "\033[44m"
C_BGPURPLE = "\033[45m"
C_BGCYAN   = "\033[46m"
C_BGWHITE  = "\033[47m"

def print_state(state):
    names = ['mx_sz', 'q_sz', 'm_cnt', 'minTh', 'maxTh', 'btnBw', 'btnDy']
    print(C_YELLOW + 'cur_state: ' + C_END, end='')
    print('[', end='')
    for i in range(6):
        print_item(names[i], str(state[i]));
    print(C_GREEN + names[6] + '=' + C_END + str(state[6]) + ']')

def print_item(name, item):
    print(C_GREEN + name + '=' + C_END + item + ', ', end='')

def do_job(subject, item):
    print(C_YELLOW + subject + ': ' + C_END, end='')
    print(item)
    return

def do_line():
    print(C_CYAN + '------------------------------------------------------' + C_END)
    return

def plotLearningPerformance(title, sub1, arr1, sub2, arr2, x):
    print("Plot Learning Performance")
    mpl.rcdefaults()
    mpl.rcParams.update({'font.size': 16})

    fig = plt.figure(figsize=(10,10))
    ax1 = fig.add_subplot(2,1,1)
    ax2 = fig.add_subplot(2,1,2)

    ax1.plot(range(len(arr1)), arr1, label=sub1, marker="", linestyle="-")#, color='k')
    ax1.set_xlabel(x)
    ax1.set_ylabel(sub1)
    ax1.set_title( title + sub1)
    ax1.grid(True, linestyle='--')
    ax1.legend(prop={'size': 12})

    ax2.plot(range(len(arr2)), arr2, label=sub2, marker="", linestyle="-")#, color='red')
    ax2.set_xlabel(x)
    ax2.set_ylabel(sub2)
    ax2.set_title(title + sub2)
    ax2.grid(True, linestyle='--')
    ax2.legend(prop={'size': 12})

    #plt.savefig('learning.pdf', bbox_inches='tight')
    plt.tight_layout()
    plt.show()

def plotLearningPerformance2(title, sub1, arr1, sub2, arr2, sub3, arr3, x):
    print("Plot Learning Performance")
    mpl.rcdefaults()
    mpl.rcParams.update({'font.size': 16})

    fig = plt.figure(figsize=(15,15))
    ax1 = fig.add_subplot(3,1,1)
    ax2 = fig.add_subplot(3,1,2)
    ax3 = fig.add_subplot(3,1,3)

    ax1.plot(range(len(arr1)), arr1, label=sub1, marker="", linestyle="-")#, color='k')
    ax1.set_ylabel(sub1)
    ax1.set_title(title)
    ax1.grid(True, linestyle='--')
    ax1.legend(prop={'size': 12})

    ax2.plot(range(len(arr2)), arr2, label=sub2, marker="", linestyle="-")#, color='red')
    ax2.set_ylabel(sub2)
    ax2.grid(True, linestyle='--')
    ax2.legend(prop={'size': 12})

    ax3.plot(range(len(arr3)), arr3, label=sub3, marker="", linestyle="-")#, color='red')
    ax3.set_xlabel(x)
    ax3.set_ylabel(sub3)
    ax3.grid(True, linestyle='--')
    ax3.legend(prop={'size': 12})

    #plt.savefig('learning.pdf', bbox_inches='tight')
    plt.tight_layout()
    plt.show()
