
<h1> Q1 STLB Prefetcher </h1>

![image](https://github.com/user-attachments/assets/58d44360-4a31-4f4a-ba5d-30c65652d4f4)

![image](https://github.com/user-attachments/assets/50aa943c-15af-4b37-82ae-b48c62e546b4)


<p> The prefetcher works by tracking the stride, which is the difference between consecutive memory addresses accessed by each instruction pointer (IP). When it identifies a consistent stride pattern with enough confidence, it proactively issues prefetch requests for future addresses based on that stride. The prefetch degree (PREFETCH_DEGREE) determines how many lines ahead it will prefetch.

Looking at the results, we see that as the prefetch degree increases from 3 to 15, the system is able to issue more prefetches. This leads to improved IPC (instructions per cycle) because more data is readily available when needed. However, there's a downside: the MPKI (misses per thousand instructions) also rises, particularly at higher degrees like 9 and 15. This increase in MPKI occurs because too many unnecessary prefetches can clutter the cache, causing useful data to be evicted. Essentially, there’s a trade-off: while we gain better data availability and higher IPC with prefetching, we also risk greater cache pollution, which can hurt performance</p>



<h1> Q2 L1D Prefetcher </h1>

<h2> IP Stride </h2>

![image](https://github.com/user-attachments/assets/2c2a9b94-d3e3-4920-b634-4deb3cb836e5)

![image](https://github.com/user-attachments/assets/077dca7d-1eee-4ef2-b07d-ed4200ef5b95)


<p> The IP_STRIDE PREFETCHER keeps an eye on memory accesses by tracking the stride patterns related to each instruction pointer (IP). When it identifies a consistent stride, it proactively requests data prefetches based on that stride, allowing the CPU to load data into the cache before it's actually needed. The function also fine-tunes its confidence in stride predictions based on how accurate those predictions have been in the past.

Looking at the results, we see that as the prefetch degree increases from 3 to 15, the MPKI (misses per thousand instructions) rises slightly. This suggests that while the prefetcher is loading more data into the cache, some of it may not be utilized effectively. On the other hand, IPC (instructions per cycle) shows a notable improvement, especially at lower degrees, indicating that the prefetcher does a good job of making data available when needed. This points to a trade-off: although higher prefetch degrees can lead to better performance, they can also cause cache inefficiency, resulting in higher MPKI and diminishing returns on IPC gains.

</p>









<h2> Complex Stride </h2>

![image](https://github.com/user-attachments/assets/064a4c5d-833c-411f-88e0-c49ca6b63348)

![image](https://github.com/user-attachments/assets/945ccd38-2bf2-49cb-9fa2-17f38f820b6b)



<p>
The complex stride (CPLX) prefetcher plays a key role in improving memory access efficiency by keeping track of how data is accessed through each instruction pointer (IP). It generates an n-bit signature that captures the most recent strides, which helps it predict future memory accesses. This signature is then used to look up the complex stride prediction table (CSPT). When the prefetcher recognizes a stride pattern, it updates a confidence counter that guides whether to prefetch future data based on the predicted strides.

However, as the prefetch degree increases, the MPKI (misses per thousand instructions) also tends to go up, which indicates that while the system is bringing in more data ahead of time, not all of it is being used effectively. Although the IPC (instructions per cycle) improves at lower prefetch degrees, this improvement starts to level off or even decline at higher degrees. This highlights a balance: while prefetching can enhance data availability, it can also lead to inefficiencies in cache usage if not managed carefully.
</p>







<h1> Q3 Optimized Prefetcher </h1>

The Optimized Prefetcher dynamically adjusts the prefetching strategy based on observed memory access patterns. It begins by tracking the number of prefetch requests issued and switches between different prefetcher types (IP-Stride, Complex Stride, and Next-Line) in phases.

During the operation, if a stride pattern is detected and the confidence level is sufficient, it issues prefetch requests to bring in future cache lines. The function also maintains counters to evaluate the effectiveness of the different prefetchers, allowing it to adapt its approach based on which method yields the best performance during the training phase. This flexibility helps optimize cache efficiency and improves overall system performance.








<h2> For Trace 1 </h2>

![image](https://github.com/user-attachments/assets/c4ad63ba-318d-4739-9544-7ceaee3e1d31)

![image](https://github.com/user-attachments/assets/3fd89612-c089-4412-9c18-062936b18dbc)


<h2> For Trace 2 </h2>

![image](https://github.com/user-attachments/assets/9c90b8d0-efef-498d-baf7-33ed24ee15e0)

![image](https://github.com/user-attachments/assets/b5158008-d74f-48e3-945a-6d59eee34ada)

<h2> For Trace 3 </h2>

![image](https://github.com/user-attachments/assets/ec957186-7d67-41b4-a5ba-44ca33d9c114)

![image](https://github.com/user-attachments/assets/ae6a7096-c1a3-4af2-9529-e7dfb18445c3)




