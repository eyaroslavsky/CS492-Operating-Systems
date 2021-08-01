"I pledge my honor that I have abided by the Stevens Honor System."

Name: Edward Yaroslavsky

Repo: https://github.com/stevens-cs492/cs-492-2021s-assignment3-eyaroslavsky.git

The first part of the bonus task d) was also implemented, where a userspace program (src/test.c) was created for testing.

Describe synchronization primitives used.

In this assignment, I used 2 semaphores, one to block the producer when the queue is full, and one to block the consumer when the queue is empty. Moreover, I used a mutex in between these semaphores to protect buffer operations, avoiding races. (Slide set 14, pages 24-25)
