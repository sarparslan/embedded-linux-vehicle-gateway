#include "ThreadSafeQueue.hpp"

#include <optional>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

TEST(ThreadSafeQueueTest, PopsInFifoOrder)
{
    ThreadSafeQueue<int> queue;
    queue.push(1);
    queue.push(2);
    queue.push(3);

    EXPECT_EQ(queue.waitAndPop(), 1);
    EXPECT_EQ(queue.waitAndPop(), 2);
    EXPECT_EQ(queue.waitAndPop(), 3);
}

TEST(ThreadSafeQueueTest, DrainsRemainingItemsAfterShutdown)
{
    ThreadSafeQueue<int> queue;
    queue.push(1);
    queue.push(2);
    queue.shutdown();
    queue.push(3);

    EXPECT_EQ(queue.waitAndPop(), 1);
    EXPECT_EQ(queue.waitAndPop(), 2);
    EXPECT_EQ(queue.waitAndPop(), std::nullopt);
}

TEST(ThreadSafeQueueTest, ShutdownWakesBlockedConsumer)
{
    ThreadSafeQueue<int> queue;
    std::optional<int> result = 0;

    std::thread consumer([&queue, &result]() {
        result = queue.waitAndPop();
    });

    queue.shutdown();
    consumer.join();

    EXPECT_EQ(result, std::nullopt);
}

TEST(ThreadSafeQueueTest, DeliversEveryItemAcrossThreads)
{
    constexpr int kItemsPerProducer = 10000;
    constexpr int kProducers = 4;

    ThreadSafeQueue<int> queue;
    std::vector<std::thread> producers;

    for (int p = 0; p < kProducers; ++p)
    {
        producers.emplace_back([&queue]() {
            for (int i = 1; i <= kItemsPerProducer; ++i)
            {
                queue.push(i);
            }
        });
    }

    long long sum = 0;
    int count = 0;

    std::thread consumer([&queue, &sum, &count]() {
        while (std::optional<int> value = queue.waitAndPop())
        {
            sum += *value;
            ++count;
        }
    });

    for (std::thread& producer : producers)
    {
        producer.join();
    }

    queue.shutdown();
    consumer.join();

    const long long expectedSum =
        static_cast<long long>(kProducers) * kItemsPerProducer * (kItemsPerProducer + 1) / 2;

    EXPECT_EQ(count, kProducers * kItemsPerProducer);
    EXPECT_EQ(sum, expectedSum);
}
