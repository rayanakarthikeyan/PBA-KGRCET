fetch("data/data.json")
  .then(res => res.json())
  .then(data => {
    const ctx = document.getElementById("chart");
    new Chart(ctx, {
      type: "line",
      data: {
        labels: data.map(row => row.LoadFactor),
        datasets: [
          {
            label: "Linear Probing",
            data: data.map(row => row.LinearProbing),
            borderWidth: 2,
            borderColor: "red"
          },
          {
            label: "Quadratic Probing",
            data: data.map(row => row.QuadraticProbing),
            borderWidth: 2,
            borderColor: "blue"
          },
          {
            label: "Chaining",
            data: data.map(row => row.Chaining),
            borderWidth: 2,
            borderColor: "green"
          }
        ]
      }
    });
  });
