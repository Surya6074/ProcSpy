async function getData() {
  const url = "/data";
  try {
    const response = await fetch(url);
    if (!response.ok) {
        throw new Error(`Response status: ${response.status}`);
    }

    const res = await response.json();

    document.getElementById("cpu").textContent = `${res.usages.cpu || 0}%`;
    document.getElementById("memory_free").textContent = `${res.usages.memory.free_mb || 0}%`;
    document.getElementById("memory_usage").textContent = `${res.usages.memory.usage || 0}%`;
    document.getElementById("total").textContent = `${res.processes.length || 0}`;


    const tbody = document.querySelector("#process-table tbody");
    tbody.innerHTML = "";

    const processes = res.processes ?? [];
    processes.forEach(p => {
        const row = document.createElement("tr");
        row.innerHTML = `
        <td>${p.pid}</td>
        <td>${p.ppid}</td>
        <td>${p.threads}</td>
        <td class="status-${p.state}">${p.state}</td>
        <td>${p.priority}</td>
        <td>${p.vmsize_mb} MB</td>
        <td class="${parseInt(p.vmrss_mb) > 100 ? 'high-mem' : ''}">${p.vmrss_mb} MB</td>
        <td>${p.user}</td>
        <td>${p.command}</td>
        <td class="align-right">${p.cpu_usage}%</td>
        <td class="align-right">${p.mem_usage}%</td>
        <td>${p.time}</td>
        `;
        tbody.appendChild(row);
    });

  } catch (error) {
    console.error(error.message);
  } finally {
    setTimeout(getData, 1);
  }
}

window.onload = () => {
  getData();
};