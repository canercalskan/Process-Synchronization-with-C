# Process-Synchronization-with-C
->The project aims at implementing a multithreaded application that handles basic synchronization problems emerging from concurrent access to shared objects:
✓ race conditions, and
✓ inter-thread coordination.
Project testbed will consist of the threads that implement following transaction categories.
✓ pay – emulates periodic deposits of the salary to a bank account.
✓ atm – sketches withdrawal or deposit transactions from/to a bank account.
✓ bill – emulates payment order withdrawals from the bank account.
✓ arch – mimics the validation and archiving of previous transactions.
->The testbed will run only 1 instance for each transaction category.


Race conditions
✓ pay – atm – bill threads race to access the account object; whereas
✓ pay – atm – bill – log threads race to access the buffer object.

Coordination
✓ consumer thread (arch) should coordinate its operations with producer threads pay – atm – bill.

The program;
→ uses POSIX Pthread API to create threads, and
→ resolves synchronization problems (race conditions and coordination) with POSIX semaphores.
