import React, { useMemo } from 'react'
import { Link } from 'react-router-dom'

// Small mock datasets — these should be replaced/wired to your store or APIs
const sampleInventory = [
  { sku: 'A-100', name: 'Widget A', quantity: 120, reorderPoint: 50 },
  { sku: 'B-200', name: 'Widget B', quantity: 8, reorderPoint: 20 },
  { sku: 'C-300', name: 'Gadget C', quantity: 45, reorderPoint: 40 },
]

const sampleOrders = [
  { id: 'PO-1', type: 'restock', sku: 'A-100', qty: 100, status: 'Pending', createdAt: new Date().toISOString() },
  { id: 'SO-1', type: 'outgoing', sku: 'E-500', qty: 20, status: 'Shipped', createdAt: new Date().toISOString() },
]

const Dashboard = ({ inventory = sampleInventory, orders = sampleOrders }) => {
  const kpis = useMemo(() => {
    const totalSKUs = inventory.length
    const totalUnits = inventory.reduce((s, i) => s + (i.quantity || 0), 0)
    const lowStock = inventory.filter(i => (i.quantity ?? 0) <= (i.reorderPoint ?? 0)).length
    const pendingPOs = orders.filter(o => o.type === 'restock' && o.status === 'Pending').length
    return { totalSKUs, totalUnits, lowStock, pendingPOs }
  }, [inventory, orders])

  return (
    <div>
      <div className="flex items-center justify-between">
        <h1 className="text-4xl font-bold">Dashboard</h1>
        <div className="text-sm text-gray-500">Overview</div>
      </div>

      <div className="grid grid-cols-1 md:grid-cols-4 gap-4 mt-6">
        <div className="p-4 bg-white rounded shadow-sm">
          <div className="text-sm text-gray-500">Total SKUs</div>
          <div className="text-2xl font-bold">{kpis.totalSKUs}</div>
        </div>
        <div className="p-4 bg-white rounded shadow-sm">
          <div className="text-sm text-gray-500">Total Units</div>
          <div className="text-2xl font-bold">{kpis.totalUnits}</div>
        </div>
        <div className="p-4 bg-white rounded shadow-sm">
          <div className="text-sm text-gray-500">Low Stock</div>
          <div className="text-2xl font-bold">{kpis.lowStock}</div>
        </div>
        <div className="p-4 bg-white rounded shadow-sm">
          <div className="text-sm text-gray-500">Pending POs</div>
          <div className="text-2xl font-bold">{kpis.pendingPOs}</div>
        </div>
      </div>

      <div className="mt-8 grid grid-cols-1 md:grid-cols-3 gap-6">
        <div className="col-span-2 bg-white p-4 rounded shadow-sm">
          <h3 className="text-xl font-semibold">Recent Orders</h3>
          {orders.length === 0 ? (
            <p className="mt-2 text-gray-500">No recent orders.</p>
          ) : (
            <ul className="mt-3 space-y-2">
              {orders.slice(0, 6).map(o => (
                <li key={o.id} className="p-3 border rounded flex justify-between">
                  <div>
                    <div className="font-medium">{o.id} • {o.sku}</div>
                    <div className="text-sm text-gray-500">{o.type} • Qty: {o.qty} • {o.status}</div>
                  </div>
                  <div className="text-sm text-gray-400">{new Date(o.createdAt).toLocaleString()}</div>
                </li>
              ))}
            </ul>
          )}
        </div>

        <div className="bg-white p-4 rounded shadow-sm">
          <h3 className="text-xl font-semibold">Warehouse Trends</h3>
          <div className="mt-3">
            {/* Simple aggregated incoming vs outgoing chart (text + bars) */}
            <div className="text-sm text-gray-500">Incoming vs Outgoing (last 30 days)</div>
            <div className="mt-2">
              {/* build sample aggregates from orders */}
              {(() => {
                const incoming = orders.filter(o => o.type === 'restock').reduce((s, o) => s + (o.qty || 0), 0)
                const outgoing = orders.filter(o => o.type === 'outgoing').reduce((s, o) => s + (o.qty || 0), 0)
                const max = Math.max(1, incoming, outgoing)
                const incPct = Math.round((incoming / max) * 100)
                const outPct = Math.round((outgoing / max) * 100)
                return (
                  <div>
                    <div className="mt-2">
                      <div className="flex items-center justify-between text-xs text-gray-600"><span>Incoming</span><span>{incoming}</span></div>
                      <div className="h-3 bg-gray-100 rounded mt-1 overflow-hidden">
                        <div style={{ width: `${incPct}%` }} className="h-full bg-green-400" />
                      </div>
                    </div>
                    <div className="mt-3">
                      <div className="flex items-center justify-between text-xs text-gray-600"><span>Outgoing</span><span>{outgoing}</span></div>
                      <div className="h-3 bg-gray-100 rounded mt-1 overflow-hidden">
                        <div style={{ width: `${outPct}%` }} className="h-full bg-red-400" />
                      </div>
                    </div>
                  </div>
                )
              })()}
            </div>

            <div className="mt-4">
              <div className="text-sm text-gray-500">Top low-stock SKUs</div>
              {(() => {
                const low = inventory
                  .filter(i => (i.quantity ?? 0) <= (i.reorderPoint ?? 0))
                  .sort((a, b) => a.quantity - b.quantity)
                  .slice(0, 5)
                if (low.length === 0) return <div className="mt-2 text-gray-500">All SKUs healthy</div>
                return (
                  <ul className="mt-2 space-y-1 text-sm">
                    {low.map(i => (
                      <li key={i.sku} className="flex justify-between"><span>{i.sku} • {i.name}</span><span className="text-gray-600">{i.quantity}</span></li>
                    ))}
                  </ul>
                )
              })()}
            </div>
          </div>
        </div>
      </div>
    </div>
  )
}

export default Dashboard
