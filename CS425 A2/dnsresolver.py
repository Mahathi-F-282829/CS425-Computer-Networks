import dns.message
import dns.query
import dns.rdatatype
import dns.resolver
import time

# Root DNS servers used to start the iterative resolution process.
# These are well-known root servers that serve as the entry point to the DNS resolution process.
# The DNS resolution will start by querying one of these root servers to find the next step in resolving a domain name.
ROOT_SERVERS = {
    "198.41.0.4": "Root (a.root-servers.net)",
    "199.9.14.201": "Root (b.root-servers.net)",
    "192.33.4.12": "Root (c.root-servers.net)",
    "199.7.91.13": "Root (d.root-servers.net)",
    "192.203.230.10": "Root (e.root-servers.net)"
}

TIMEOUT = 3  # Timeout in seconds for each DNS query attempt to prevent indefinite waiting.

def send_dns_query(server, domain):
    """ 
    Sends a DNS query to the specified server for an A record (IPv4 address) of the given domain.
    - A DNS query is constructed using `dns.message.make_query()`.
    - The query is sent to the specified server using the UDP protocol, as it is more efficient for DNS lookups.
    - A timeout is set to prevent delays if the server does not respond.
    - If the query is successful, the function returns the response.
    - If there is a failure (e.g., timeout, unreachable server), None is returned to indicate failure.
    """
    try:
        query = dns.message.make_query(domain, dns.rdatatype.A)  # Construct a DNS query for the domain
        response = dns.query.udp(query, server, timeout=TIMEOUT)  # Send the query via UDP with a timeout
        return response  # Return the response received from the server
    except Exception:
        return None  # If an error occurs (timeout, unreachable server, etc.), return None

def extract_next_nameservers(response):
    """ 
    Extracts NS (Name Server) records from the authority section of the response.
    - When a nameserver does not have the final answer, it provides a referral to another nameserver.
    - The response may contain authoritative NS (Name Server) records, pointing to the next step in the resolution.
    - These NS records contain hostnames, which need to be resolved into IP addresses before they can be queried.
    - The function extracts NS records and attempts to resolve them into usable IP addresses.
    - Returns a list of resolved IP addresses of the next authoritative nameservers.
    """
    ns_ips = []  # List to store resolved nameserver IPs
    ns_names = []  # List to store nameserver domain names
    
    # Check if the response contains an authority section, which might contain NS records.
    if response.authority:
        for rrset in response.authority:
            if rrset.rdtype == dns.rdatatype.NS:  # Check if the record type is NS (Name Server)
                for rr in rrset:
                    ns_name = rr.to_text()  # Extract nameserver domain name
                    ns_names.append(ns_name)  # Store NS domain name for resolution
                    print(f"Extracted NS hostname: {ns_name}")
    
    # Resolve extracted NS hostnames to IP addresses, as DNS queries need IPs rather than domain names.
    for ns_name in ns_names:
        try:
            answer = dns.resolver.resolve(ns_name, "A")  # Resolve NS hostname to an IPv4 address
            for rdata in answer:
                ns_ips.append(rdata.to_text())  # Store the resolved IP address
        except Exception:
            continue  # Skip if resolution fails
    
    return ns_ips  # Return the list of resolved nameserver IPs to be used for further querying.

def iterative_dns_lookup(domain):
    """ 
    Performs iterative DNS resolution starting from root servers.
    - The process starts at the root DNS servers and follows referrals down the hierarchy.
    - It first queries a root DNS server, which provides a referral to the relevant Top-Level Domain (TLD) server.
    - Then, the TLD server provides a referral to the authoritative server for the domain.
    - The process continues until the final authoritative nameserver provides an answer or the lookup fails.
    """
    print(f"[Iterative DNS Lookup] Resolving {domain}")

    next_ns_list = list(ROOT_SERVERS.keys())  # Initialize with root server IPs
    stage = "ROOT"  # Track resolution stage (ROOT -> TLD -> AUTH)

    while next_ns_list:
        ns_ip = next_ns_list[0]  # Pick the first available nameserver
        response = send_dns_query(ns_ip, domain)  # Query the nameserver
        
        if response:
            print(f"[DEBUG] Querying {stage} server ({ns_ip}) - SUCCESS")
            
            # If an answer is found, print and return
            if response.answer:
                print(f"[SUCCESS] {domain} -> {response.answer[0][0]}")
                return
            
            # Extract the next set of nameservers if no final answer found
            next_ns_list = extract_next_nameservers(response)
            
            # Move to the next resolution stage (TLD or AUTH)
            stage = "TLD" if stage == "ROOT" else "AUTH"
        else:
            print(f"[ERROR] Query failed for {stage} {ns_ip}")
            return  # Stop resolution if query fails
    
    print("[ERROR] Resolution failed.")  # Print failure message if no NS responds

def recursive_dns_lookup(domain):
    """ 
    Performs recursive DNS resolution using the system's default resolver.
    - Unlike iterative resolution, where the resolver manually follows referrals,
      recursive resolution delegates the entire process to an external DNS resolver.
    - The system's configured resolver (e.g., Google DNS, ISP-provided resolver) handles the lookup.
    - This method is simpler but relies on an external resolver instead of manually walking the DNS hierarchy.
    """
    print(f"[Recursive DNS Lookup] Resolving {domain}")
    try:
        answer = dns.resolver.resolve(domain, "A")  # Perform recursive resolution
        for rdata in answer:
            print(f"[SUCCESS] {domain} -> {rdata}")  # Print resolved IP
    except Exception as e:
        print(f"[ERROR] Recursive lookup failed: {e}")  # Handle resolution failure

if __name__ == "__main__":
    import sys  # Import sys to read command-line arguments
    
    # Validate input arguments
    if len(sys.argv) != 3 or sys.argv[1] not in {"iterative", "recursive"}:
        print("Usage: python3 dns_resolver.py <iterative|recursive> <domain>")
        sys.exit(1)  # Exit if incorrect arguments are provided

    mode = sys.argv[1]  # Get mode (iterative or recursive)
    domain = sys.argv[2]  # Get domain to resolve
    start_time = time.time()  # Record start time
    
    # Execute the selected DNS resolution mode
    if mode == "iterative":
        iterative_dns_lookup(domain)
    else:
        recursive_dns_lookup(domain)
    
    # Print execution time
    print(f"Time taken: {time.time() - start_time:.3f} seconds")
