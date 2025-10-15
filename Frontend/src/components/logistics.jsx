import React, { useMemo } from 'react'

// Mock inventory data — replace with real API/store as needed


const smallCard = (title, value, note) => (
  <div className="p-4 bg-white rounded-lg shadow-sm">
    <div className="text-sm text-gray-500">{title}</div>
    <div className="text-2xl font-bold">{value}</div>
    {note && <div className="text-xs text-gray-400">{note}</div>}
  </div>
)

const Logistics = () => {
  const [inventory, setInventory] = React.useState([]);
  const [loading, setLoading] = React.useState();

  React.useEffect(() => {
    const loadData = async () => {
      try {
        setLoading(true);
        const res = await fetch('http://localhost:8080/inventory');
        const data = await res.json();
        setInventory(data);
      } catch (err) {
        console.error('Failed to fetch inventory data:', err);
      } finally {
        setLoading(false);
      }
    }
    loadData();
  }, [])

  // Derived KPIs
  const kpis = useMemo(() => {
    const totalSKUs = inventory.length
    const totalUnits = inventory.reduce((s, it) => s + (it.quantity || 0), 0)
    const lowStock = inventory.filter(it => (it.quantity ?? 0) <= (it.reorder ?? 0))

    return { totalSKUs, totalUnits, lowStock }
  }, [inventory])

  if (loading) {
    return <div className="loader"></div>
  }

  return (
    <div>
      <h2 className="text-4xl font-bold">Logistics</h2>
      <p className="mt-4 text-gray-600">Overview of warehouse health and logistics KPIs.</p>

      <div className="grid grid-cols-1 md:grid-cols-3 gap-4 mt-6">
        {smallCard('Total SKUs', kpis.totalSKUs)}
        {smallCard('Total Units', kpis.totalUnits)}
        {smallCard('Low / Reorder', `${kpis.lowStock.length}`, 'Items at or below reorder point')}
      </div>

      <div className="mt-8">
        <h3 className="text-2xl font-semibold">Low stock items</h3>
        {kpis.lowStock.length === 0 ? (
          <p className="mt-2 text-gray-500">All items are above reorder points.</p>
        ) : (
          <table className="w-full mt-4 table-auto border-collapse">
            <thead>
              <tr className="text-left border-b">
                <th className="py-2">SKU</th>
                <th className="py-2">Name</th>
                <th className="py-2">Quantity</th>
                <th className="py-2">Reorder Point</th>
                <th className="py-2">Incoming</th>
                <th className="py-2">Action</th>
              </tr>
            </thead>
            <tbody>
              {kpis.lowStock.map(it => (
                <tr key={it.sku} className="border-b">
                  <td className="py-2">{it.sku}</td>
                  <td className="py-2">{it.name}</td>
                  <td className="py-2">{it.quantity}</td>
                  <td className="py-2">{it.reorder}</td>
                  <td className="py-2">{it.incoming}</td>
                  <td className="py-2">
                    <button className="bg-blue-500 text-white px-2 py-1 rounded">Create PO</button>
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        )}
      </div>
    </div>
  )
}

export default Logistics
