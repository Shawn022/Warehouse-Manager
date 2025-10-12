import React, { useMemo } from 'react'

// Mock inventory data — replace with real API/store as needed
const sampleInventory = [
  { sku: 'A-100', name: 'Widget A', quantity: 120, reorderPoint: 50, incoming: 30, outgoingLast30Days: 20 },
  { sku: 'B-200', name: 'Widget B', quantity: 8, reorderPoint: 20, incoming: 0, outgoingLast30Days: 15 },
  { sku: 'C-300', name: 'Gadget C', quantity: 45, reorderPoint: 40, incoming: 10, outgoingLast30Days: 5 },
  { sku: 'D-400', name: 'Part D', quantity: 0, reorderPoint: 10, incoming: 100, outgoingLast30Days: 2 },
  { sku: 'E-500', name: 'Thing E', quantity: 300, reorderPoint: 100, incoming: 0, outgoingLast30Days: 60 },
]

const smallCard = (title, value, note) => (
  <div className="p-4 bg-white rounded-lg shadow-sm">
    <div className="text-sm text-gray-500">{title}</div>
    <div className="text-2xl font-bold">{value}</div>
    {note && <div className="text-xs text-gray-400">{note}</div>}
  </div>
)

const Logistics = ({ inventory = sampleInventory }) => {
  // Derived KPIs
  const kpis = useMemo(() => {
    const totalSKUs = inventory.length
    const totalUnits = inventory.reduce((s, it) => s + (it.quantity || 0), 0)
    const lowStock = inventory.filter(it => (it.quantity ?? 0) <= (it.reorderPoint ?? 0))
    const incomingUnits = inventory.reduce((s, it) => s + (it.incoming || 0), 0)
    const outgoing30 = inventory.reduce((s, it) => s + (it.outgoingLast30Days || 0), 0)
    const avgDaysToDeplete = (() => {
      // naive estimate: if outgoing rate per 30 days is 0, treat as very large
      const depletedSkus = inventory.filter(it => (it.outgoingLast30Days || 0) > 0)
      if (depletedSkus.length === 0) return '—'
      const days = depletedSkus.map(it => {
        const daily = (it.outgoingLast30Days || 0) / 30
        return (it.quantity || 0) / daily
      })
      const avg = Math.round(days.reduce((a, b) => a + b, 0) / days.length)
      return isFinite(avg) ? `${avg} days` : '—'
    })()

    return { totalSKUs, totalUnits, lowStock, incomingUnits, outgoing30, avgDaysToDeplete }
  }, [inventory])

  return (
    <div>
      <h2 className="text-4xl font-bold">Logistics</h2>
      <p className="mt-4 text-gray-600">Overview of warehouse health and logistics KPIs.</p>

      <div className="grid grid-cols-1 md:grid-cols-3 gap-4 mt-6">
        {smallCard('Total SKUs', kpis.totalSKUs)}
        {smallCard('Total Units', kpis.totalUnits)}
        {smallCard('Low / Reorder', `${kpis.lowStock.length}`, 'Items at or below reorder point')}
        {smallCard('Incoming Units', kpis.incomingUnits)}
        {smallCard('Outgoing (30d)', kpis.outgoing30)}
        {smallCard('Est. Days to Deplete', kpis.avgDaysToDeplete)}
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
                  <td className="py-2">{it.reorderPoint}</td>
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
