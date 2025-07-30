function loadFakeData() {
    const cpuUsage = (Math.random() * 100).toFixed(1);
    const memUsage = (Math.random() * 100).toFixed(1);

    document.getElementById("cpu").textContent = `${cpuUsage}%`;
    document.getElementById("memory").textContent = `${memUsage}%`;

    const processes = [
        { pid: 123, ppid: 1, threads: 4, status: "S", vmsize: "50MB", vmrss: "10MB", comm: "bash" },
        { pid: 234, ppid: 123, threads: 2, status: "R", vmsize: "200MB", vmrss: "180MB", comm: "firefox" },
        { pid: 789, ppid: 1, threads: 1, status: "Z", vmsize: "10MB", vmrss: "5MB", comm: "defunct" },
        { pid: 999, ppid: 2, threads: 3, status: "T", vmsize: "70MB", vmrss: "65MB", comm: "gdb" }
    ];

    document.getElementById("total").textContent = processes.length;

    const tbody = document.querySelector("#process-table tbody");
    tbody.innerHTML = "";

    processes.forEach(p => {
        const row = document.createElement("tr");
        row.innerHTML = `
        <td>${p.pid}</td>
        <td>${p.ppid}</td>
        <td>${p.threads}</td>
        <td class="status-${p.status}">${p.status}</td>
        <td>${p.vmsize}</td>
        <td class="${parseInt(p.vmrss) > 100 ? 'high-mem' : ''}">${p.vmrss}</td>
        <td>${p.comm}</td>
        `;
        tbody.appendChild(row);
    });
}

document.querySelectorAll("th").forEach(th => {
    th.addEventListener("click", () => {
        console.log("Sorting column:", th.innerText);
    });
});

window.onload = () => {
    loadFakeData();
    setInterval(loadFakeData, 5000);
};
