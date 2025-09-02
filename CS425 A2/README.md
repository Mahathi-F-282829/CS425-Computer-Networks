# DNS Resolver Tool

This Python script provides a tool for resolving domain names into IP addresses using both **iterative** and **recursive** lookup methods. It leverages the `dnspython` library to perform direct queries to DNS servers (iterative) or use the system's default resolver (recursive).

## Features

- **Iterative DNS Lookup**: Manually resolves a domain by querying each level of the DNS hierarchy, starting from the root servers, moving to top-level domain (TLD) servers, and finally querying the authoritative nameservers.
- **Recursive DNS Lookup**: Uses the system's default resolver to handle the resolution process internally, returning the final resolved IP address.

## Prerequisites

Ensure you have the following installed:

- **Python 3.x**
- **dnspython library**, which can be installed using:

  ```bash
  pip install dnspython
  ```

## How to Run the Code

This script requires two command-line arguments:
1. The DNS resolution mode (`iterative` or `recursive`).
2. The domain name to resolve.

### Running the Script

#### Iterative DNS Lookup
To perform an iterative lookup for `example.com`, use:

```bash
python3 dns_resolver.py iterative example.com
```

#### Recursive DNS Lookup
To perform a recursive lookup for `example.com`, use:

```bash
python3 dns_resolver.py recursive example.com
```

## How It Works

### Iterative DNS Lookup
1. The script starts by querying a **root DNS server**.
2. The root server provides a referral to the next level, such as a **TLD (Top-Level Domain) server**.
3. The TLD server then refers the query to the **authoritative nameserver** responsible for the domain.
4. The process continues until a nameserver provides the final IP address or the resolution process fails.
5. The script outputs the resolved IP address and debug messages for each step.

### Recursive DNS Lookup
1. The script sends the query to the system's **default DNS resolver** (e.g., an ISP resolver or a public DNS like Google 8.8.8.8).
2. The resolver performs the entire resolution process and returns the resolved IP address.
3. The script outputs the resolved IP address or an error message if the lookup fails.

## Output

- If the resolution is successful, the script prints one or more IP addresses associated with the domain.
- **Iterative lookups** display debugging information about each nameserver queried during the resolution process.
- The script also prints the total **execution time** to indicate lookup performance.

## Notes

- **Iterative lookups** provide a transparent step-by-step resolution process, which is useful for understanding how DNS works.
- **Recursive lookups** are quicker since they rely on an external resolver.
- If a domain cannot be resolved, the script will display an appropriate error message.

This tool is designed for learning and testing DNS resolution mechanics. Use it to explore how domain names are mapped to IP addresses across the internet.