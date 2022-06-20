module.exports = async ({getNamedAccounts, deployments}) => {
    const {deploy} = deployments;
    const {deployer} = await getNamedAccounts();


    const bridge = await deploy('NativeOracleBridge', {
        from: deployer,
        args: [1],
    });

    console.log("Deployed to:", bridge.address);

};
module.exports.tags = ['NativeOracleBridge'];