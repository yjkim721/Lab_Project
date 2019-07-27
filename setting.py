# ns3-env Args
port = 5555
simTime = 100 # seconds
stepTime = 0.1  # seconds
seed = 0
simArgs = {"--simTime": simTime,
           "--testArg": 123}
debug = False

# Learning Agent Args
ACTOR_LEARNING_RATE = 0.001
CRITIC_LEARNING_RATE = 0.0001
GAMMA = 0.99
TAU = 0.005
MEM_SIZE = 100000

START_MEM = 5000
N_H1 = 400
N_H2 = 300

STATE_SIZE = 7
ACTION_SIZE = 1
BATCH_SIZE = 10
MAX_STEPS = int(simTime / stepTime)
FAIL_PENALTY = 0
ACTION_RANGE = 1
EVALUATE_EVERY = 10

NUM_EPISODES = 1
